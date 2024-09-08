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
	class NewSceneDialog : public UI::Developer::Dialog
	{
	protected:
		Dialog::Result internal_draw(UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;
	private:
		NewSceneDialog();

		const std::string& get_new_scene_name() const
		{
			return m_scene_name.input;
		}

		bool has_valid_name() const
		{
			return m_scene_name.input.empty() == false;
		}

		UI::Developer::Button m_create_button;
		UI::Developer::Button m_cancel_button;

		UI::Developer::InputText m_scene_name;

		friend class SceneTreeWindow;
	};

	class LoadSceneDialog : public UI::Developer::Dialog
	{
	protected:
		LoadSceneDialog(Core::Editor& editor, std::string_view title);

		Dialog::Result internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui) override;

		void on_open() override;

		Core::Editor& m_editor;

		UI::Developer::ListBox m_scene_item_list;
		VadonEditor::Model::SceneList m_scene_list;

		UI::Developer::Button m_load_button;
		UI::Developer::Button m_cancel_button;

	private:
		LoadSceneDialog(Core::Editor& editor);

		Vadon::Scene::ResourceHandle get_loaded_scene() const
		{
			return m_scene_list[m_scene_item_list.selected_item].scene_handle;
		}

		bool has_valid_option() const
		{
			return m_scene_list.empty() == false;
		}

		friend class SceneTreeWindow;
	};
}
#endif