#include <VadonEditor/View/Scene/Resource/Scene.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/View/ViewSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <filesystem>

namespace VadonEditor::View
{
	UI::Developer::Dialog::Result SaveSceneDialog::internal_draw(UI::Developer::GUISystem& dev_gui)
	{
		Result result = Result::NONE;

		Vadon::Utilities::Vector2 file_browser_area = dev_gui.get_available_content_region();
		const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();

		file_browser_area.y -= dev_gui.calculate_text_size(m_scene_name.label).y + gui_style.frame_padding.y * 2;
		file_browser_area.y -= dev_gui.calculate_text_size(m_save_button.label).y + gui_style.frame_padding.y * 2 + 10.0f;

		m_file_browser.draw(dev_gui);
		if (m_file_browser.get_selected_file().double_clicked == true)
		{
			// File double clicked, set its name in the input
			m_scene_name.input = std::filesystem::path(m_file_browser.get_selected_file().path).stem().string();
		}

		dev_gui.draw_input_text(m_scene_name);

		if (dev_gui.draw_button(m_save_button) == true)
		{
			// TODO: error dialog if not valid!
			if (validate_path() == true)
			{
				result = Result::ACCEPTED;
				close();
			}
		}
		dev_gui.same_line();
		if (dev_gui.draw_button(m_cancel_button) == true)
		{
			result = Result::CANCELLED;
			close();
		}
		return result;
	}

	SaveSceneDialog::SaveSceneDialog(Core::Editor& editor)
		: Dialog("Save Scene As")
		, m_editor(editor)
	{
		m_scene_name.label = "Scene Name";

		m_save_button.label = "Save";
		m_cancel_button.label = "Cancel";
	}

	std::string SaveSceneDialog::get_save_path() const
	{
		return (std::filesystem::path(m_file_browser.get_current_path()) / (m_scene_name.input + ".vdsc")).string();
	}

	bool SaveSceneDialog::validate_path() const
	{
		// TODO: any other validation?
		if (m_scene_name.input.empty() == true)
		{
			return false;
		}

		return true;
	}

	void SaveSceneDialog::save_scene_as(Model::Scene* scene)
	{
		m_scene = scene;
		const Vadon::Scene::ResourceInfo& scene_info = scene->get_info();
		Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
		if (scene_info.path.is_valid() == true)
		{
			// Existing scene
			m_file_browser.navigate_to_path(file_system.get_absolute_path(scene_info.path));
			m_scene_name.input = std::filesystem::path(scene_info.path.path).stem().string();
		}
		else
		{
			// New scene, need to set path
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
			m_file_browser.navigate_to_path(file_system.get_absolute_path(Vadon::Core::FileSystemPath{ .root_directory = project_manager.get_active_project().root_dir_handle, .path = ""}));
			m_scene_name.input.clear();
		}

		open();
	}

	SelectSceneDialog::SelectSceneDialog(Core::Editor& editor, std::string_view title)
		: Dialog(title)
		, m_editor(editor)
	{
		m_select_button.label = "Select";
		m_cancel_button.label = "Cancel";

		m_scene_list_box.label = "#scene_list";
	}

	UI::Developer::Dialog::Result SelectSceneDialog::internal_draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		// TODO: implement search field
		Result result = Result::NONE;

		m_scene_list_box.size = dev_gui.get_available_content_region();
		const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();

		m_scene_list_box.size.y -= dev_gui.calculate_text_size(m_select_button.label).y + gui_style.frame_padding.y * 2 + 5.0f;

		dev_gui.draw_list_box(m_scene_list_box);
		if (has_valid_option() == false)
		{
			dev_gui.begin_disabled();
		}
		if (dev_gui.draw_button(m_select_button) == true)
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

	void SelectSceneDialog::on_open()
	{
		// Rebuild scene list
		m_scene_list.clear();
		m_scene_list_box.items.clear();
		m_scene_list_box.deselect();

		Model::ModelSystem& model = m_editor.get_system<Model::ModelSystem>();
		Model::SceneSystem& scene_system = model.get_scene_system();

		std::vector<Model::SceneInfo> model_scene_list = scene_system.get_scene_list();

		for (const Model::SceneInfo& current_scene_info : model_scene_list)
		{
			m_scene_list.push_back(current_scene_info.handle);
			m_scene_list_box.items.push_back(current_scene_info.info.path.path);
		}
	}

	SceneListWindow::SceneListWindow(Core::Editor& editor)
		: m_editor(editor)
		, m_save_scene_dialog(editor)
		, m_load_scene_dialog(editor, "Load Scene")
	{
		m_window.title = "Scene List";
		m_window.open = false;

		m_scene_list_box.label = "#scene_list";
		m_scene_list_box.size = { -1, -1 };
	}

	void SceneListWindow::draw(UI::Developer::GUISystem& dev_gui)
	{
		if (dev_gui.begin_window(m_window) == true)
		{
			if (m_save_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
			{
				// TODO: prompt if about overwrite?
				Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
				Vadon::Core::FileSystem& file_system = m_editor.get_engine_core().get_system<Vadon::Core::FileSystem>();
				const Vadon::Core::RootDirectoryHandle project_root_dir = project_manager.get_active_project().root_dir_handle;

				const Vadon::Scene::ResourcePath new_path = { .root_directory = project_root_dir, .path = file_system.get_relative_path(m_save_scene_dialog.get_save_path(), project_root_dir) };
				m_save_scene_dialog.m_scene->set_path(new_path);
				if (m_save_scene_dialog.m_scene->save() == true)
				{
					const int32_t scene_index = get_scene_index(m_save_scene_dialog.m_scene);
					// TODO: assert?
					m_scene_list_box.items[scene_index] = m_save_scene_dialog.m_scene->get_info().path.path;
				}
				else
				{
					m_editor.get_engine_core().log_error("SceneListWindow: failed to save Scene!\n");
				}
			}

			if (m_load_scene_dialog.draw(dev_gui) == VadonApp::UI::Developer::Dialog::Result::ACCEPTED)
			{
				Model::ModelSystem& model_system = m_editor.get_system<Model::ModelSystem>();
				Model::SceneSystem& scene_system = model_system.get_scene_system();

				Model::Scene* loaded_scene = scene_system.get_scene(m_load_scene_dialog.get_selected_scene());
				if (loaded_scene != nullptr)
				{
					open_scene(loaded_scene);
				}
				else
				{
					m_editor.get_engine_core().log_error("SceneListWindow: failed to load Scene!\n");
				}
			}

			dev_gui.draw_list_box(m_scene_list_box);
			if (m_scene_list_box.has_valid_selection() == true)
			{
				update_active_scene(m_scene_list[m_scene_list_box.selected_item]);
			}
		}
		dev_gui.end_window();
	}

	void SceneListWindow::on_new_scene_action()
	{
		// Create a new scene and open it
		Model::ModelSystem& model_system = m_editor.get_system<Model::ModelSystem>();
		Model::SceneSystem& scene_system = model_system.get_scene_system();

		Model::Scene* new_scene = scene_system.create_scene();
		if (new_scene == nullptr)
		{
			m_editor.get_engine_core().log_error("SceneListWindow: failed to create new Scene!\n");
			return;
		}

		open_scene(new_scene);
	}

	void SceneListWindow::on_save_scene_action()
	{
		ViewModel& view_model = m_editor.get_system<ViewSystem>().get_view_model();
		Model::Scene* active_scene = view_model.get_active_scene();

		if (active_scene->is_modified() == false)
		{
			return;
		}

		if (active_scene->get_info().path.is_valid() == false)
		{
			// New scene, have to set path
			m_save_scene_dialog.save_scene_as(active_scene);
		}
		else
		{
			// Existing scene, attempt to save to previous path
			if (active_scene->save() == false)
			{
				m_editor.get_engine_core().log_error("SceneListWindow: failed to save Scene!\n");
			}
		}
	}

	void SceneListWindow::on_load_scene_action()
	{
		// TODO: only open if no other dialog is open?
		// TODO2: prompt user if there are unsaved changes in scene?
		m_load_scene_dialog.open();
	}

	void SceneListWindow::open_scene(Model::Scene* scene)
	{
		int32_t scene_index = get_scene_index(scene);
		if (scene_index < 0)
		{
			// Add scene to list
			scene_index = static_cast<int32_t>(m_scene_list.size());
			m_scene_list_box.selected_item = scene_index;
			m_scene_list.push_back(scene);

			const Vadon::Scene::ResourceInfo& scene_info = scene->get_info();
			if (scene_info.path.is_valid() == true)
			{
				m_scene_list_box.items.push_back(scene_info.path.path);
			}
			else
			{
				m_scene_list_box.items.push_back("<New Scene>");
			}
		}

		Model::ModelSystem& model = m_editor.get_system<Model::ModelSystem>();
		Model::SceneSystem& scene_system = model.get_scene_system();
		scene_system.open_scene(scene);

		m_scene_list_box.selected_item = scene_index;
		update_active_scene(scene);
	}

	void SceneListWindow::update_active_scene(Model::Scene* scene)
	{
		ViewModel& view_model = m_editor.get_system<ViewSystem>().get_view_model();
		view_model.set_active_scene(scene);
	}

	int32_t SceneListWindow::get_scene_index(Model::Scene* scene) const
	{
		int32_t scene_index = 0;
		for (Model::Scene* current_scene : m_scene_list)
		{
			if (current_scene == scene)
			{
				return scene_index;
			}

			++scene_index;
		}

		return -1;
	}
}