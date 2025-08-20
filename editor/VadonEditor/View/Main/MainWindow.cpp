#include <VadonEditor/View/Main/MainWindow.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/View/Project/AssetBrowser.hpp>
#include <VadonEditor/View/Project/Project.hpp>

#include <VadonEditor/View/Scene/Animation/AnimationEditor.hpp>
#include <VadonEditor/View/Scene/ECS/Entity.hpp>
#include <VadonEditor/View/Scene/Resource/Resource.hpp>
#include <VadonEditor/View/Scene/Resource/Scene.hpp>
#include <VadonEditor/View/Scene/SceneTree.hpp>

namespace VadonEditor::View
{
	struct MainWindow::Internal
	{
		Core::Editor& m_editor;

		// FIXME: implement menu API with actions?
		// Scene menu
		UI::Developer::MenuItem m_new_scene_menu;
		UI::Developer::MenuItem m_load_scene_menu;
		UI::Developer::MenuItem m_save_scene_menu;

		// Project menu
		UI::Developer::MenuItem m_project_settings_menu;
		UI::Developer::MenuItem m_export_project_menu;
		UI::Developer::MenuItem m_close_project_menu;

		ProjectLauncher m_launcher;
		UI::Developer::FileBrowserDialog m_export_project_browser;
		ProjectPropertiesDialog m_project_properties;

		AssetBrowser m_asset_browser;

		SceneListWindow m_scene_list;
		SceneTreeWindow m_scene_tree;
		EntityEditor m_entity_editor;
		ResourceEditor m_resource_editor;
		AnimationEditor m_animation_editor;

		Internal(Core::Editor& editor)
			: m_editor(editor)
			, m_launcher(editor)
			, m_project_properties(editor)
			, m_asset_browser(editor)
			, m_scene_list(editor)
			, m_scene_tree(editor)
			, m_entity_editor(editor)
			, m_resource_editor(editor)
			, m_animation_editor(editor)
		{
			// FIXME: use pooled strings so we don't need to cache!
			m_new_scene_menu.label = "New Scene";
			m_load_scene_menu.label = "Load Scene";
			m_save_scene_menu.label = "Save Scene";

			m_project_settings_menu.label = "Project Settings";
			m_export_project_menu.label = "Export Project";
			m_close_project_menu.label = "Close Project";
		}

		bool initialize()
		{
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
			if (project_manager.get_state() == Core::ProjectManager::State::LAUNCHER)
			{
				m_launcher.initialize();
			}

			return true;
		}

		void draw(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();

			if (project_manager.get_state() == Core::ProjectManager::State::PROJECT_LOADED)
			{
				draw_project_widgets(dev_gui);
			}
			else
			{
				// No active project, show the launcher
				m_launcher.draw(dev_gui);
			}
		}

		void draw_project_widgets(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			// TODO: use some kind of "state enum" for the editor to decide what the main window should show?
			// Could be relevant when handling editor settings, etc.
			draw_main_menu(dev_gui);
			m_asset_browser.draw(dev_gui);
			m_scene_list.draw(dev_gui);
			m_scene_tree.draw(dev_gui);
			m_entity_editor.draw(dev_gui);
			m_resource_editor.draw(dev_gui);
			m_animation_editor.draw(dev_gui);
		}

		void draw_main_menu(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			// FIXME: should instead have these be actions (inspired by Qt) where we can register callbacks
			if (dev_gui.begin_main_menu_bar() == true)
			{
				if (dev_gui.begin_menu("Scene") == true)
				{
					if (dev_gui.add_menu_item(m_new_scene_menu) == true)
					{
						m_scene_list.on_new_scene_action();
					}
					if (dev_gui.add_menu_item(m_load_scene_menu) == true)
					{
						m_scene_list.on_load_scene_action();
					}
					dev_gui.add_separator();
					if (dev_gui.add_menu_item(m_save_scene_menu) == true)
					{
						m_scene_list.on_save_scene_action();
					}
					dev_gui.end_menu();
				}

				if (dev_gui.begin_menu("Project") == true)
				{
					if (dev_gui.add_menu_item(m_project_settings_menu) == true)
					{
						m_project_properties.open();
					}
					if (dev_gui.add_menu_item(m_export_project_menu) == true)
					{
						Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
						m_export_project_browser.open_at(project_manager.get_active_project().info.root_path);
					}
					if (dev_gui.add_menu_item(m_close_project_menu) == true)
					{
						// FIXME: check for unsaved changes, etc!
						Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
						project_manager.close_project();
					}
					dev_gui.end_menu();
				}

				dev_gui.end_main_menu_bar();
			}

			if (m_project_properties.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
				project_manager.update_project_custom_properties(m_project_properties.get_edited_properties());
			}

			if (m_export_project_browser.draw(dev_gui) == UI::Developer::Dialog::Result::ACCEPTED)
			{
				Core::ProjectManager& project_manager = m_editor.get_system<Core::ProjectManager>();
				project_manager.export_project(m_export_project_browser.get_current_path());
			}
		}
	};

	MainWindow::~MainWindow() = default;

	MainWindow::MainWindow(Core::Editor& editor)
		: m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool MainWindow::initialize()
	{
		return m_internal->initialize();
	}

	void MainWindow::draw(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		m_internal->draw(dev_gui);
	}
}