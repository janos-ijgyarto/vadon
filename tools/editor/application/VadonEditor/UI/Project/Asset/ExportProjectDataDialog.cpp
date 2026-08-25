#include <VadonEditor/UI/Project/Asset/ExportProjectDataDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Core/Asset/AssetServer.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Plugin.hpp>

#include <QFileDialog>

namespace VadonEditor::UI
{	
	ExportProjectDataBackend::ExportProjectDataBackend(Core::Application& application, const QString& output_path)
		: m_application(application)
		, m_output_path(output_path)
	{
	}

	bool ExportProjectDataBackend::initialize()
	{
		connect(&m_application.get_network_system(), &Network::NetworkSystem::received_message, this, &ExportProjectDataBackend::received_message);
		return true;
	}

	void ExportProjectDataBackend::received_message(const QByteArray& data)
	{
		::Vadon::Foundation::EditorMessageReader message_reader(data.constData(), data.size());

		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::PLUGIN:
		{
			const char* message_data = message_reader.get_current_message_data();
			const ::Vadon::Foundation::EditorPluginMessageHeader* plugin_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorPluginMessageHeader*>(message_data);
			switch (plugin_message_header->message_type)
			{
			case ::Vadon::Foundation::EditorPluginMessageType::PLUGIN_INIT:
			{
				const ::Vadon::Foundation::EditorPluginMessageInit* plugin_init_message = reinterpret_cast<const ::Vadon::Foundation::EditorPluginMessageInit*>(message_reader.get_current_message_data());
				if (plugin_init_message->error_code != 0)
				{
					cleanup();
					return;
				}

				switch (plugin_message_header->plugin_type)
				{
				case ::Vadon::Foundation::EditorPluginMessageSource::ASSET_SERVER:
				{
					Core::AssetServer& asset_server = m_application.get_asset_server();
					asset_server.export_project_data(m_output_path);
					cleanup();
				}
				break;
				}
			}
			break;
			}
		}
		break;
		}
	}

	void ExportProjectDataBackend::cleanup()
	{
		deleteLater();
	}

	ExportProjectDataDialog::ExportProjectDataDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
	{
		m_ui.setupUi(this);

		// Initialize with the loaded project info
		const Core::ProjectInfo project_info = m_application.get_project_manager().get_project_info();
		Q_ASSERT_X(project_info.plugin_settings.selected_configuration.isEmpty() == false, "VadonEditor::UI::ExportProjectDataDialog::ExportProjectDataDialog", "No valid config!");

		m_ui.configurationValueLabel->setText(project_info.plugin_settings.selected_configuration);

		update_controls();
	}

	void ExportProjectDataDialog::accept()
	{
		const QString output_path = m_ui.outputPathLineEdit->text();
		if (output_path.isEmpty() == true)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ExportProjectDataDialog::accept", "Did not select destination!");
			return;
		}

		Core::AssetServer& asset_server = m_application.get_asset_server();
		if (asset_server.is_running() == false)
		{
			// Create a backend to run the export once the asset server is ready
			// FIXME: should we instead start the asset server as soon as the editor is open?
			ExportProjectDataBackend* export_backend = new ExportProjectDataBackend(m_application, output_path);
			if (export_backend->initialize() == false)
			{
				export_backend->deleteLater();
				return;
			}

			Core::AssetServerSettings asset_server_settings;
			asset_server_settings.debug_break_on_init = true;

			const Core::ProjectInfo project_info = m_application.get_project_manager().get_project_info();
			asset_server_settings.configuration_name = project_info.plugin_settings.selected_configuration;

			asset_server.run_asset_server(asset_server_settings);
		}
		else
		{
			// Asset server already running, export the data
			asset_server.export_project_data(output_path);
		}

		QDialog::accept();
	}

	void ExportProjectDataDialog::browse_clicked()
	{
		const Core::ProjectInfo project_info = m_application.get_project_manager().get_project_info();
		const QString selected_path = QFileDialog::getExistingDirectory(this, "Select Output Folder", project_info.root_path);
		if (selected_path.isEmpty() == false)
		{
			// TODO: run any checks on whether the path is valid?
			m_ui.outputPathLineEdit->setText(selected_path);
			update_controls();
		}
	}

	void ExportProjectDataDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		if (m_ui.outputPathLineEdit->text().isEmpty() == true)
		{
			ok_button->setEnabled(false);
		}
		else
		{
			ok_button->setEnabled(true);
		}
	}
}