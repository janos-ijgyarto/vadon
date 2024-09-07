#include <VadonEditor/View/Scene/Resource/Scene.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result NewSceneDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;
		dev_gui.draw_input_text(m_scene_name);

		if (has_valid_name() == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_create_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}
		if (has_valid_name() == false)
		{
			dev_gui.end_disabled();
		}
		dev_gui.same_line();
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
			close();
		}
		return result;
	}
	
	void NewSceneDialog::on_open()
	{
		m_scene_name.input.clear();
	}

	NewSceneDialog::NewSceneDialog()
		: Dialog("Create New Scene")
	{
		m_create_button.label = "Create";
		m_cancel_button.label = "Cancel";

		m_scene_name.label = "Scene name";
	}

	LoadSceneDialog::LoadSceneDialog(Core::Editor& editor, std::string_view title)
		: Dialog(title)
		, m_editor(editor)
	{
		m_load_button.label = "Load";
		m_cancel_button.label = "Cancel";

		m_scene_item_list.label = "Scenes";
	}

	UI::Developer::Dialog::Result LoadSceneDialog::internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		dev_gui.draw_list_box(m_scene_item_list);
		if (has_valid_option() == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_load_button) == true)
		{
			result = Result::ACCEPTED;
			close();
		}
		if (has_valid_option() == false)
		{
			dev_gui.end_disabled();
		}
		dev_gui.same_line();
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
			close();
		}
		return result;
	}

	void LoadSceneDialog::on_open()
	{
		// Rebuild scene list
		Model::ModelSystem & model = m_editor.get_system<Model::ModelSystem>();
		Model::SceneTree& scene_tree = model.get_scene_tree();

		m_scene_item_list.items.clear();
		m_scene_list = scene_tree.get_scene_list();

		// Exclude current scene
		Vadon::Scene::ResourceID current_scene_id;
		if (scene_tree.get_current_scene().is_valid() == true)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
			current_scene_id = resource_system.get_resource_id(scene_tree.get_current_scene());
		}

		for (auto scene_it = m_scene_list.begin(); scene_it != m_scene_list.end(); ++scene_it)
		{
			if (scene_it->scene_id == current_scene_id)
			{
				m_scene_list.erase(scene_it);
				break;
			}
		}

		for (const Model::Scene& current_scene : m_scene_list)
		{
			m_scene_item_list.items.push_back(current_scene.resource_path.path);
		}

		m_scene_item_list.selected_item = 0;
	}

	LoadSceneDialog::LoadSceneDialog(Core::Editor& editor)
		: LoadSceneDialog(editor, "Load Scene")
	{}
}