#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Logger.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/UI/Model/Animation/AnimationEditor.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Scene/SceneDialog.hpp>

#include <VadonEditor/UI/Project/DataSchemaDialog.hpp>
#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>

#include <VadonEditor/UI/Utilities/UUIDDialog.hpp>

#include <QFileDialog>
#include <QMessageBox>

namespace VadonEditor::UI
{
	MainWindow::MainWindow(Core::Application& application, QWidget* parent)
		: QMainWindow(parent)
		, m_application(application)
	{
		m_ui.setupUi(this);

		QObject::connect(&m_application.get_logger(), &Core::Logger::message_logged, this, &UI::MainWindow::message_logged);
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::message_logged(QtMsgType type, const QString& message)
	{
		// FIXME: this assumes dark mode
		// We should toggle based on app style setting!
		QString html_color = "white";
		switch (type)
		{
		case QtDebugMsg:
			html_color = "pink";
			break;
		case QtWarningMsg:
			html_color = "yellow";
			break;
		case QtCriticalMsg:
			html_color = "red";
			break;
		case QtFatalMsg:
			html_color = "darkRed";
			break;
		}
		m_ui.console->appendHtml(QString("<p style=\"color:%1;white-space:pre\">%2</p>").arg(html_color).arg(message.simplified()));
	}

	void MainWindow::new_resource_triggered()
	{
		// NOTE: we can fire-and-forget this object, it will clean itself up when the dialog closes
		new NewResourceDialogBackend(m_application, this);
	}

	void MainWindow::new_scene_triggered()
	{
		NewSceneDialog* new_scene_dialog = new NewSceneDialog(m_application, this);
		new_scene_dialog->open();
	}

	void MainWindow::save_triggered()
	{
		Model::ModelSystem& model_system = m_application.get_model_system();
		model_system.get_scene_system().save_all_scenes();
		model_system.get_resource_system().save_all_resources();
	}

	void MainWindow::open_triggered()
	{
		OpenAssetDialog* open_asset_dialog = new OpenAssetDialog(m_application, this);
		connect(open_asset_dialog, &OpenAssetDialog::asset_opened, this, &MainWindow::asset_opened);

		open_asset_dialog->open();
	}

	void MainWindow::import_triggered()
	{
		const QString selected_file = QFileDialog::getOpenFileName(this, "Select File To Import", m_application.get_project_manager().get_project_info().root_path, QString(), nullptr, QFileDialog::Option::ReadOnly);
		if (selected_file.isEmpty() == false)
		{
			Core::AssetManager& asset_manager = m_application.get_asset_manager();
			const QModelIndex imported_asset_index = asset_manager.import_asset_file(selected_file);
			if (imported_asset_index.isValid() == false)
			{
				QMessageBox::critical(this, "Asset Manager Error", "Failed to import file!");
				return;
			}

			const Core::AssetInfo asset_info = asset_manager.get_asset_info(imported_asset_index);
			Model::Resource* imported_resource = m_application.get_model_system().get_resource_system().import_file_resource(asset_info.id);
			if (imported_resource == nullptr)
			{
				QMessageBox::critical(this, "Resource Manager Error", "Failed to create import resource!");
				return;
			}
		}
	}

	void MainWindow::quit_triggered()
	{
		close();
	}

	void MainWindow::generate_uuid_triggered()
	{
		UUIDDialog* uuid_dialog = new UUIDDialog(this);
		uuid_dialog->open();
	}

	void MainWindow::project_settings_triggered()
	{
		ProjectSettingsDialog* project_settings_dialog = new ProjectSettingsDialog(m_application, this);
		project_settings_dialog->open();
	}

	void MainWindow::run_plugin_triggered()
	{
		emit run_simulator_requested();
	}

	void MainWindow::shutdown_plugin_triggered()
	{
		emit stop_simulator_requested();
	}

	void MainWindow::project_data_schema_triggered()
	{
		DataSchemaDialog* data_schema_dialog = new DataSchemaDialog(m_application, this);
		data_schema_dialog->open();
	}

	void MainWindow::generate_data_schema_triggered()
	{
		Core::ProjectManager& project_manager = m_application.get_project_manager();
		if (project_manager.generate_project_data_schema() == false)
		{
			QMessageBox::critical(this, "Project manager error", "Failed to generate project data schema!");
		}

		if(project_manager.load_project_data_schema() == false)
		{
			QMessageBox::critical(this, "Project manager error", "Failed to load project data schema!");
		}
	}

	void MainWindow::anim_editor_prototype_triggered()
	{
		AnimationEditor* anim_editor = new AnimationEditor();
		anim_editor->setWindowTitle("Animation Editor Prototype");
		anim_editor->show();
	}

	void MainWindow::asset_opened(const QString& asset_path)
	{
		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		const QModelIndex asset_index = asset_manager.find_asset_index_by_path(asset_path);
		if (asset_index.isValid() == false)
		{
			QMessageBox::critical(this, "Asset Library Error", QString("Cannot find asset at \"%1\"").arg(asset_path));
			return;
		}

		asset_manager.open_asset(asset_index);
	}

	void MainWindow::closeEvent(QCloseEvent* event)
	{
		if (m_application.get_ui_system().is_shutting_down() == false)
		{
			// Emit signal so UI system decides if we are ready to shut down
			emit(close_requested());

			// Ignore event so we don't actually close
			event->ignore();
			return;
		}

		event->accept();
	}
}