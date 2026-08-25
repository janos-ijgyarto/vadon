#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadoneditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QFileDialog>
#include <QMessageBox>

namespace
{
	// TODO: move this to common utility header?
	void set_line_edit_no_signal(QLineEdit* line_edit, const QString& text)
	{
		line_edit->blockSignals(true);
		line_edit->setText(text);
		line_edit->blockSignals(false);
	}
}

namespace VadonEditor::UI
{
	ProjectSettingsDialog::ProjectSettingsDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_custom_data_resource_editor(nullptr)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		// Initialize with the loaded project info
		m_project_info = m_application.get_project_manager().get_project_info();

		m_ui.customDataToolButton->addAction(m_ui.actionLoadResource);
		m_ui.customDataToolButton->addAction(m_ui.actionClear);

		m_ui.nameValueLabel->setText(m_project_info.name);
		m_ui.rootPathValueLabel->setText(m_project_info.root_path);

		update_custom_data_resource_widget();
		
		set_line_edit_no_signal(m_ui.pluginCustomPathEdit, m_project_info.plugin_settings.custom_search_path);
		reset_plugin_config_combo();

		set_line_edit_no_signal(m_ui.gameCustomPathEdit, m_project_info.game_settings.custom_search_path);
		reset_game_config_combo();
	}

	void ProjectSettingsDialog::accept()
	{
		m_application.get_project_manager().set_project_info(m_project_info);
		QDialog::accept();
	}

	void ProjectSettingsDialog::load_custom_data_resource_triggered()
	{
		SelectResourceDialog* select_dialog = new SelectResourceDialog(m_application, Model::Resource::get_base_resource_type(), this);
		connect(select_dialog, &SelectResourceDialog::resource_asset_selected, this, &ProjectSettingsDialog::custom_data_resource_selected);

		select_dialog->open();
	}

	void ProjectSettingsDialog::clear_custom_data_resource_triggered()
	{
		custom_data_resource_selected(QUuid());
	}

	void ProjectSettingsDialog::custom_data_resource_selected(const QUuid& resource_id)
	{
		m_project_info.custom_data_resource_id = resource_id;
		update_custom_data_resource_widget();
	}

	void ProjectSettingsDialog::plugin_custom_path_browse_clicked()
	{
		const QString selected_dir = QFileDialog::getExistingDirectory(this, "Select Editor Plugin Custom Search Path", m_application.get_project_manager().get_project_info().root_path);
		if (selected_dir.isEmpty() == false)
		{
			m_ui.pluginCustomPathEdit->setText(selected_dir);
		}
	}

	void ProjectSettingsDialog::plugin_custom_path_clear_clicked()
	{
		m_ui.pluginCustomPathEdit->clear();
	}

	void ProjectSettingsDialog::plugin_custom_path_text_changed(const QString& text)
	{
		m_project_info.plugin_settings.custom_search_path = text;
		m_project_info.plugin_settings.selected_configuration.clear();

		update_editor_plugin_list();
	}

	void ProjectSettingsDialog::plugin_configuration_activated(int index)
	{
		m_project_info.plugin_settings.selected_configuration = m_ui.pluginConfigurationCombo->itemText(index);
	}

	void ProjectSettingsDialog::game_custom_path_browse_clicked()
	{
		const QString selected_dir = QFileDialog::getExistingDirectory(this, "Select Game Executable Custom Search Path", m_application.get_project_manager().get_project_info().root_path);
		if (selected_dir.isEmpty() == false)
		{
			m_ui.gameCustomPathEdit->setText(selected_dir);
		}
	}

	void ProjectSettingsDialog::game_custom_path_clear_clicked()
	{
		m_ui.gameCustomPathEdit->clear();
	}

	void ProjectSettingsDialog::reset_plugin_config_combo()
	{
		m_ui.pluginConfigurationCombo->clear();

		if (m_project_info.plugin_entries.isEmpty())
		{
			return;
		}

		int selected_index = 0;
		for (int current_index = 0; current_index < m_project_info.plugin_entries.count(); ++current_index)
		{
			const Core::EditorPluginInfo& plugin_info = m_project_info.plugin_entries[current_index];
			m_ui.pluginConfigurationCombo->addItem(plugin_info.configuration_name);

			if (plugin_info.configuration_name == m_project_info.plugin_settings.selected_configuration)
			{
				selected_index = current_index;
			}
		}

		m_ui.pluginConfigurationCombo->setCurrentIndex(selected_index);
	}

	void ProjectSettingsDialog::reset_game_config_combo()
	{
		m_ui.gameConfigurationCombo->clear();

		if (m_project_info.game_entries.isEmpty())
		{
			return;
		}

		int selected_index = 0;
		for (int current_index = 0; current_index < m_project_info.game_entries.count(); ++current_index)
		{
			const Core::GameExecutableInfo& game_info = m_project_info.game_entries[current_index];
			m_ui.gameConfigurationCombo->addItem(game_info.configuration_name);

			if (game_info.configuration_name == m_project_info.game_settings.selected_configuration)
			{
				selected_index = current_index;
			}
		}

		m_ui.gameConfigurationCombo->setCurrentIndex(selected_index);
	}

	void ProjectSettingsDialog::game_custom_path_text_changed(const QString& text)
	{
		m_project_info.game_settings.custom_search_path = text;
		m_project_info.game_settings.selected_configuration.clear();

		update_game_executable_list();
	}

	void ProjectSettingsDialog::game_configuration_activated(int index)
	{
		m_project_info.game_settings.selected_configuration = m_ui.gameConfigurationCombo->itemText(index);
	}

	void ProjectSettingsDialog::update_custom_data_resource_widget()
	{
		int global_config_row = 0;
		m_ui.generalTabForm->getWidgetPosition(m_ui.customDataLabel, &global_config_row, nullptr);
		
		Q_ASSERT_X(global_config_row >= 0, "VadonEditor::UI::ProjectSettingsDialog::update_custom_data_resource_widget", "Cannot find custom data row!");

		if (Utilities::is_uuid_valid(m_project_info.custom_data_resource_id) == true)
		{
			if (m_custom_data_resource_editor != nullptr)
			{
				if (m_custom_data_resource_editor->get_resource()->get_info().id == m_project_info.custom_data_resource_id)
				{
					// Editor already shows the same resource
					return;
				}
				else
				{
					clear_custom_data_resource_widget();
				}
			}

			Model::Resource* resource = m_application.get_model_system().get_resource_system().get_resource(m_project_info.custom_data_resource_id);
			if (resource == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ProjectSettingsDialog::update_global_config_widget", "Cannot find resource!");
				return;
			}

			m_custom_data_resource_editor = new ResourceEditor(resource, this);
			if (m_custom_data_resource_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ProjectSettingsDialog::update_global_config_widget", "Failed to initialize resource editor!");
				m_custom_data_resource_editor->deleteLater();
				m_custom_data_resource_editor = nullptr;
				return;
			}

			m_custom_data_resource_editor->set_read_only(true);

			m_ui.generalTabForm->insertRow(global_config_row + 1, m_custom_data_resource_editor);


			// Update label
			const Model::ResourceInfo resource_info = resource->get_info();
			const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
			const Core::TypeData* type_data = data_schema.find_type_data(resource_info.type);

			QString current_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
			if (current_type_name.isEmpty())
			{
				current_type_name = QString("Resource type %1").arg(resource_info.type.toString());
			}

			QString label_string = current_type_name;
			if (resource->is_embedded() == false)
			{
				const int resource_asset_id = m_application.get_model_system().get_resource_system().find_resource_asset_id(resource_info.id);
				Q_ASSERT_X(resource_asset_id != Core::AssetInfo::c_invalid_file_id, "VadonEditor::UI::PropertyResource::generate_resource_widgets", "Cannot find resource asset");

				Core::AssetManager& asset_manager = m_application.get_asset_manager();
				const QModelIndex asset_index = asset_manager.find_asset_index(resource_asset_id);
				const Core::AssetInfo asset_info = m_application.get_asset_manager().get_asset_info(asset_index);
				label_string += QString(" (%1) - %2").arg(Utilities::uuid_to_base64_string(resource_info.id)).arg(asset_info.path);
			}
			else
			{
				// TODO: create a "path" of resource properties for embedded resource
				label_string += QString(" (%1)").arg(Utilities::uuid_to_base64_string(resource_info.id));
			}

			m_ui.customDataHeaderLabel->setText(label_string);
		}
		else
		{
			clear_custom_data_resource_widget();
		}
	}

	void ProjectSettingsDialog::clear_custom_data_resource_widget()
	{
		if (m_custom_data_resource_editor == nullptr)
		{
			return;
		}

		m_ui.generalTabForm->removeWidget(m_custom_data_resource_editor);
		m_custom_data_resource_editor->deleteLater();
		m_custom_data_resource_editor = nullptr;

		m_ui.customDataHeaderLabel->setText("N/A");
	}

	void ProjectSettingsDialog::update_editor_plugin_list()
	{
		const QString search_path = m_project_info.plugin_settings.custom_search_path.isEmpty() ? m_project_info.root_path : m_project_info.plugin_settings.custom_search_path;

		m_project_info.plugin_entries = m_application.get_project_manager().find_editor_plugins(search_path);
		if (m_project_info.plugin_entries.isEmpty() == true)
		{
			QMessageBox::warning(this, "Editor Plugin Settings", "No plugins found at path");
		}

		reset_plugin_config_combo();
	}

	void ProjectSettingsDialog::update_game_executable_list()
	{
		const QString search_path = m_project_info.game_settings.custom_search_path.isEmpty() ? m_project_info.root_path : m_project_info.game_settings.custom_search_path;

		m_project_info.game_entries = m_application.get_project_manager().find_game_executables(search_path);
		if (m_project_info.game_entries.isEmpty() == true)
		{
			QMessageBox::warning(this, "Game Settings", "No game executables found at path");
		}

		reset_game_config_combo();
	}
}