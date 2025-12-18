#ifndef VADONEDITOR_VIEW_PROJECT_PROJECT_HPP
#define VADONEDITOR_VIEW_PROJECT_PROJECT_HPP
#include <VadonEditor/Core/Project/Project.hpp>
#include <VadonEditor/UI/Developer/Widgets.hpp>
#include <VadonEditor/View/Scene/Resource/Resource.hpp>
#include <VadonApp/UI/Developer/Widgets/FileBrowser.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	class NewProjectDialog : public UI::Developer::Dialog
	{
	protected:
		Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;
	private:
		NewProjectDialog(Core::Editor& editor);

		bool has_valid_input() const;
		bool validate() const;

		Core::Editor& m_editor;

		// TODO: other configurable parameters?
		UI::Developer::InputText m_input_name;
		UI::Developer::Button m_browse_button;
		std::string m_root_path;

		UI::Developer::FileBrowserDialog m_file_browser;

		UI::Developer::Button m_create_button;
		UI::Developer::Button m_cancel_button;

		friend class ProjectLauncher;
	};

	class ProjectLauncher
	{
	private:
		ProjectLauncher(Core::Editor& editor);
		void initialize();
		void draw(UI::Developer::GUISystem& dev_gui);

		UI::Developer::Window m_window;

		Core::Editor& m_editor;
		Core::Project m_selected_project_info;

		NewProjectDialog m_new_project_dialog;
		VadonApp::UI::Developer::FileBrowserDialog m_import_project_dialog;

		UI::Developer::Button m_new_project_button;
		UI::Developer::Button m_import_project_button;
		UI::Developer::Button m_open_project_button;
		UI::Developer::ListBox m_project_list;

		friend class MainWindow;
	};

	class ProjectPropertiesDialog : public UI::Developer::Dialog
	{
	protected:
		Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;

		const Model::Resource* get_custom_data_resource() const { return m_custom_data_editor.get_resource(); }
	private:
		ProjectPropertiesDialog(Core::Editor& editor);

		void update_label();

		Core::Editor& m_editor;

		UI::Developer::Button m_ok_button;
		UI::Developer::Button m_cancel_button;

		ResourceEditorWidget m_custom_data_editor;

		SelectResourceDialog m_select_resource_dialog;
		UI::Developer::Button m_select_resource_button;
		UI::Developer::Button m_clear_resource_button;

		std::string m_resource_label;

		friend class MainWindow;
	};
}
#endif