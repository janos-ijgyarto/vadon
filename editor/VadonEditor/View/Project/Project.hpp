#ifndef VADONEDITOR_VIEW_PROJECT_PROJECT_HPP
#define VADONEDITOR_VIEW_PROJECT_PROJECT_HPP
#include <VadonEditor/Core/Project/Project.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <VadonEditor/UI/Developer/Widgets.hpp>
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
		UI::Developer::InputText m_input_path; // FIXME: implement file browser!

		UI::Developer::Button m_create_button;
		UI::Developer::Button m_cancel_button;

		friend class ProjectLauncher;
	};

	class ImportProjectDialog : public UI::Developer::Dialog
	{
	protected:
		Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;
	private:
		ImportProjectDialog(Core::Editor& editor);

		bool has_valid_input() const;
		bool validate() const;

		Core::Editor& m_editor;

		UI::Developer::InputText m_project_path; // FIXME: implement file browser!
		UI::Developer::Button m_import_button;
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
		ImportProjectDialog m_import_project_dialog;

		UI::Developer::Button m_new_project_button;
		UI::Developer::Button m_import_project_button;
		UI::Developer::Button m_open_project_button;
		UI::Developer::ListBox m_project_list;

		friend class MainWindow;
	};
}
#endif