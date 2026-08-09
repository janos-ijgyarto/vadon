#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

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
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		// Initialize with the loaded project info
		m_project_info = m_application.get_project_manager().get_project_info();

		m_ui.nameValueLabel->setText(m_project_info.name);
		m_ui.rootPathValueLabel->setText(m_project_info.root_path);
		
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