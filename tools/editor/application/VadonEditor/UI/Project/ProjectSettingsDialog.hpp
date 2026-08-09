#ifndef VADONEDITOR_UI_PROJECT_PROJECTSETTINGSDIALOG_HPP
#define VADONEDITOR_UI_PROJECT_PROJECTSETTINGSDIALOG_HPP
#include <VadonEditor/Core/Project/Project.hpp>
#include <QDialog>
#include <VadonEditor/UI/Project/ui_ProjectSettingsDialog.h>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class ProjectSettingsDialog : public QDialog
	{
		Q_OBJECT
	public:
		ProjectSettingsDialog(Core::Application& application, QWidget* parent);

		void accept() override;
	private slots:
		void plugin_custom_path_browse_clicked();
		void plugin_custom_path_clear_clicked();
		void plugin_custom_path_text_changed(const QString& text);

		void plugin_configuration_activated(int index);

		void game_custom_path_browse_clicked();
		void game_custom_path_clear_clicked();
		void game_custom_path_text_changed(const QString& text);

		void game_configuration_activated(int index);
	private:
		void update_editor_plugin_list();
		void update_game_executable_list();

		void reset_plugin_config_combo();
		void reset_game_config_combo();

		Core::Application& m_application;
		Ui::ProjectSettings m_ui;

		Core::ProjectInfo m_project_info;
	};
}
#endif