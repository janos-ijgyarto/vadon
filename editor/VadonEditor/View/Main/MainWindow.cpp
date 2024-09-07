#include <VadonEditor/View/Main/MainWindow.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/View/Project/Project.hpp>
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
		UI::Developer::MenuItem m_close_project_menu;

		ProjectLauncher m_launcher;
		SceneTreeWindow m_scene_tree;

		Internal(Core::Editor& editor)
			: m_editor(editor)
			, m_launcher(editor)
			, m_scene_tree(editor)
		{
			// FIXME: use pooled strings so we don't need to cache!
			m_new_scene_menu.label = "New Scene";
			m_load_scene_menu.label = "Load Scene";
			m_save_scene_menu.label = "Save Scene";

			m_project_settings_menu.label = "Project Settings";
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

			if (project_manager.get_state() == Core::ProjectManager::State::PROJECT_OPEN)
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
			m_scene_tree.draw(dev_gui);
			// TODO: anything else?
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
						m_scene_tree.on_new_scene_action();
					}
					if (dev_gui.add_menu_item(m_load_scene_menu) == true)
					{
						m_scene_tree.on_load_scene_action();
					}
					dev_gui.add_separator();
					if (dev_gui.add_menu_item(m_save_scene_menu) == true)
					{
						m_scene_tree.on_save_scene_action();
					}
					dev_gui.end_menu();
				}

				if (dev_gui.begin_menu("Project") == true)
				{
					if (dev_gui.add_menu_item(m_project_settings_menu) == true)
					{
						// TODO
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