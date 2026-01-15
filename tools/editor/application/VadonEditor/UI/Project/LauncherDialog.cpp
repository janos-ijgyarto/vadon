#include <VadonEditor/UI/Project/LauncherDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/UI/Project/NewProjectDialog.hpp>

#include <QFileDialog>
#include <QMessageBox>

namespace
{
	constexpr int c_project_path_role = Qt::ItemDataRole::UserRole + 1;
}

namespace VadonEditor::UI
{
	LauncherDialog::LauncherDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_new_project_dialog(nullptr)
	{
		m_ui.setupUi(this);

		QMetaObject::invokeMethod(this, &LauncherDialog::init_ui, Qt::ConnectionType::QueuedConnection);
	}

	void LauncherDialog::init_ui()
	{
		m_ui.projectListWidget->clear();

		Core::ProjectManager& project_manager = m_application.get_project_manager();

		const QList<VadonEditor::Core::ProjectManager::CachedProjectInfo> cached_projects = project_manager.get_cached_project_list();

		for (const VadonEditor::Core::ProjectManager::CachedProjectInfo& current_project : cached_projects)
		{
			const QString project_item_label = QString("%1 (%2)").arg(current_project.name).arg(current_project.path);
			QListWidgetItem* current_item = new QListWidgetItem(project_item_label, m_ui.projectListWidget);

			current_item->setData(c_project_path_role, current_project.path);
		}
	}

	void LauncherDialog::new_clicked()
	{
		m_new_project_dialog = new NewProjectDialog(this);
		connect(m_new_project_dialog, &QDialog::accepted, this, &LauncherDialog::new_project_created);
		connect(m_new_project_dialog, &QObject::destroyed, this, &LauncherDialog::new_project_dialog_destroyed);

		m_new_project_dialog->open();
	}

	void LauncherDialog::import_clicked()
	{
		QString project_file = QFileDialog::getOpenFileName(this, "Select Project File", QDir::currentPath(), tr("Project Files (*.vdpr)"));
		if (project_file.isEmpty() == false)
		{
			if (m_application.get_project_manager().import_project(project_file) == true)
			{
				// Import successful, add to list
				init_ui();
			}
			else
			{
				QMessageBox::critical(this, "Project Manager Error", tr("Failed to import project!"));
			}
		}
	}

	void LauncherDialog::run_clicked()
	{
		run_selected_project();
	}

	void LauncherDialog::remove_clicked()
	{
		QListWidgetItem* selected_item = get_selected_item();

		// First remove from project cache
		m_application.get_project_manager().remove_project(selected_item->data(c_project_path_role).toString());

		// Remove from list widget
		const int row = m_ui.projectListWidget->row(selected_item);

		selected_item = m_ui.projectListWidget->takeItem(row);
		delete selected_item;
	}

	void LauncherDialog::project_selection_changed()
	{
		const bool any_selected = get_selected_item() != nullptr;

		m_ui.runButton->setEnabled(any_selected);
		m_ui.removeButton->setEnabled(any_selected);
	}

	void LauncherDialog::project_double_clicked(QListWidgetItem* item)
	{
		Q_UNUSED(item);
		run_selected_project();
	}

	void LauncherDialog::new_project_created()
	{
		const VadonEditor::Core::ProjectInfo new_project_info = m_new_project_dialog->get_project_info();
		if (m_application.get_project_manager().create_project(new_project_info) == true)
		{
			// Creation successful, add to list
			init_ui();
		}
		else
		{
			QMessageBox::critical(this, "Project Manager Error", tr("Failed to create new project!"));
		}
	}

	QListWidgetItem* LauncherDialog::get_selected_item() const
	{
		QList<QListWidgetItem*> selected_items = m_ui.projectListWidget->selectedItems();
		if (selected_items.isEmpty() == false)
		{
			return selected_items.first();
		}

		return nullptr;
	}

	void LauncherDialog::run_selected_project()
	{
		QListWidgetItem* selected_project_item = get_selected_item();
		if (selected_project_item == nullptr)
		{
			return;
		}

		if (m_application.get_project_manager().load_project(selected_project_item->data(c_project_path_role).toString()) == false)
		{
			QMessageBox::critical(this, "Project Manager Error", tr("Failed to load project!"));
			return;
		}

		accept();
	}
}