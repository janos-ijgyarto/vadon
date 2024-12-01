#ifndef VADONEDITOR_VIEW_SCENE_RESOURCE_SCENE_HPP
#define VADONEDITOR_VIEW_SCENE_RESOURCE_SCENE_HPP
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>
#include <VadonEditor/UI/Developer/Widgets.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::View
{
	// Basically a "Save As" dialog
	class SaveSceneDialog : public UI::Developer::Dialog
	{
	protected:
		Dialog::Result internal_draw(UI::Developer::GUISystem& dev_gui) override;
	private:
		SaveSceneDialog(Core::Editor& editor);

		std::string get_save_path() const;

		void save_scene_as(Model::Scene* scene);
		bool validate_path() const;

		Core::Editor& m_editor;

		Model::Scene* m_scene;
		UI::Developer::InputText m_scene_name;

		UI::Developer::FileBrowserWidget m_file_browser;

		UI::Developer::Button m_save_button;
		UI::Developer::Button m_cancel_button;

		friend class SceneListWindow;
	};

	class SelectSceneDialog : public UI::Developer::Dialog
	{
	public:
		Vadon::Scene::SceneHandle get_selected_scene() const
		{
			return m_scene_list[m_scene_list_box.selected_item];
		}
	protected:
		SelectSceneDialog(Core::Editor& editor, std::string_view title);

		Dialog::Result internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;

		Core::Editor& m_editor;

		std::vector<Vadon::Scene::SceneHandle> m_scene_list;

		UI::Developer::ListBox m_scene_list_box;

		UI::Developer::Button m_select_button;
		UI::Developer::Button m_cancel_button;
	private:
		bool has_valid_option() const
		{
			return m_scene_list.empty() == false;
		}

		friend class SceneListWindow;
	};

	// TODO: this is its own widget, equivalent to the tabs for scenes in Godot as it shows all the open scenes
	class SceneListWindow
	{
	private:
		SceneListWindow(Core::Editor& editor);

		void draw(UI::Developer::GUISystem& dev_gui);

		void on_new_scene_action();
		void on_save_scene_action();
		void on_load_scene_action();

		void open_scene(Model::Scene* scene);
		void update_active_scene(Model::Scene* scene);
		int32_t get_scene_index(Model::Scene* scene) const;

		Core::Editor& m_editor;

		UI::Developer::Window m_window;
		UI::Developer::ListBox m_scene_list_box;
		std::vector<Model::Scene*> m_scene_list;

		SaveSceneDialog m_save_scene_dialog;
		SelectSceneDialog m_load_scene_dialog;

		friend class MainWindow;
	};
}
#endif