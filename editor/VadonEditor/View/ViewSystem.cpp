#include <VadonEditor/View/ViewSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/Developer/GUI.hpp>

#include <VadonEditor/View/Main/MainWindow.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <format>

namespace VadonEditor::View
{
	namespace
	{		
		// FIXME: move to Application, add as option to show debug window for provided ECS world
		class ECSDebugView
		{
		public:
			ECSDebugView()
			{
				m_window.title = "ECS Debug View";
				m_window.open = false;

				m_entity_list_box.label = "Entities";
				m_update_entity_list_button.label = "Update";
			}

			void draw(Core::Editor& editor)
			{
				// Only draw if we have an active project
				if (editor.get_system<Core::ProjectManager>().get_state() != Core::ProjectManager::State::PROJECT_ACTIVE)
				{
					return;
				}

				VadonApp::Core::Application& engine_app = editor.get_engine_app();
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

				if (dev_gui.begin_window(m_window) == true)
				{
					dev_gui.draw_list_box(m_entity_list_box);

					if (dev_gui.draw_button(m_update_entity_list_button) == true)
					{
						update_entity_list(editor);
					}
				}
				dev_gui.end_window();
			}
		private:
			void update_entity_list(Core::Editor& editor)
			{
				m_entity_list_box.items.clear();

				Vadon::ECS::EntityManager& entity_manager = editor.get_system<Model::ModelSystem>().get_ecs_world().get_entity_manager();
				m_entities = entity_manager.get_active_entities();

				for (const Vadon::ECS::EntityHandle& current_entity : m_entities)
				{
					const std::string entity_label = std::format("{} ({}_{})", entity_manager.get_entity_name(current_entity), current_entity.handle.index, current_entity.handle.generation);
					m_entity_list_box.items.push_back(entity_label);
				}
			}

			UI::Developer::Window m_window;

			UI::Developer::ListBox m_entity_list_box;
			UI::Developer::Button m_update_entity_list_button;

			Vadon::ECS::EntityList m_entities;
		};
	}

	struct ViewSystem::Internal
	{
		Core::Editor& m_editor;

		MainWindow m_main_window;
		ECSDebugView m_ecs_debug_view;

		Internal(Core::Editor& editor)
			: m_editor(editor)
			, m_main_window(editor)
		{
		}

		bool initialize()
		{
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

			m_editor.get_system<UI::UISystem>().register_ui_element([this, &dev_gui](Core::Editor& editor) {
				m_main_window.draw(dev_gui);
				m_ecs_debug_view.draw(editor);
				});

			if(m_main_window.initialize() == false)
			{
				return false;
			}

			return true;
		}

		void update()
		{
			// TODO
		}
	};

	ViewSystem::~ViewSystem() = default;

	ViewSystem::ViewSystem(Core::Editor& editor)
		: m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool ViewSystem::initialize()
	{
		return m_internal->initialize();
	}

	void ViewSystem::update()
	{
		m_internal->update();
	}
}