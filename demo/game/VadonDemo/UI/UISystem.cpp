#include <VadonDemo/UI/UISystem.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/Component.hpp>
#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/UI/Component.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>

#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Data/Visitor.hpp>

namespace VadonDemo::UI
{
	UISystem::~UISystem() = default;

	void UISystem::register_dev_gui_callback(const DevGUICallback& callback)
	{
		m_dev_gui_callbacks.push_back(callback);
	}

	void UISystem::register_console_command(std::string_view command_name, const ConsoleCommandCallback& callback)
	{
		m_console_commands[std::string(command_name)] = callback;
	}

	UISystem::UISystem(Core::GameCore& core)
		: m_game_core(core)
		, m_main_window(core)
	{

	}

	bool UISystem::initialize()
	{
		// Register callback in platform interface
		m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>().register_event_callback(
			[this](const VadonApp::Platform::PlatformEventList& platform_events)
			{
				auto platform_event_visitor = Vadon::Utilities::VisitorOverloadList{
					[this](const VadonApp::Platform::QuitEvent&)
					{
						// Platform is trying to quit, so we request stop
						m_game_core.request_shutdown();
					},
					[this](const VadonApp::Platform::KeyboardEvent& keyboard_event)
					{
						switch (keyboard_event.key)
						{
						case VadonApp::Platform::KeyCode::BACKQUOTE:
						{
							m_main_window.show();
							m_dev_gui_enabled = true;
						}
						break;
						case VadonApp::Platform::KeyCode::RETURN:
						{
							if (keyboard_event.down == false && Vadon::Utilities::to_bool(keyboard_event.modifiers & VadonApp::Platform::KeyModifiers::LEFT_ALT))
							{
								m_game_core.get_platform_interface().toggle_fullscreen();
							}
						}
						break;
						case VadonApp::Platform::KeyCode::ESCAPE:
						{
							m_dev_gui_enabled = false;
						}
						break;
						}
						
					},
					[](auto) { /* Default, do nothing */ }
				};

				for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
				{
					std::visit(platform_event_visitor, current_event);
				}
			}
		);

		// Handle console events
		// TODO: dispatch console events properly!
		{
			VadonApp::UI::Console& console = m_game_core.get_engine_app().get_system<VadonApp::UI::UISystem>().get_console();
			console.register_event_handler(
				[this, &console](const VadonApp::UI::ConsoleCommandEvent& command_event)
				{
					if (command_event.text.empty() == true)
					{
						return false;
					}

					const size_t command_name_end = command_event.text.find(" ", 0);
					std::string_view command_name = command_event.text;
					std::string_view command_args = "";
					if (command_name_end != std::string::npos)
					{
						command_args = command_name.substr(command_name_end + 1);
						command_name = command_name.substr(0, command_name_end);
					}		

					auto command_it = m_console_commands.find(std::string(command_name));
					if (command_it == m_console_commands.end())
					{
						console.log_error(std::format("Command not recognized: \"{}\"\n", command_name));
						return false;
					}

					// Run callback
					command_it->second(command_args);
					return true;
				}
			);
		}

		init_dev_gui();

		if (m_main_window.initialize() == false)
		{
			return false;
		}

		init_game_ui();

		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		component_manager.register_event_callback<VadonDemo::UI::Base>(
			[this](const Vadon::ECS::ComponentEvent& component_event)
			{
				switch (component_event.event_type)
				{
				case Vadon::ECS::ComponentEventType::ADDED:
					m_deferred_init_queue.push_back(component_event.owner);
					break;
				case Vadon::ECS::ComponentEventType::REMOVED:
					remove_entity(component_event.owner);
					break;
				}
			}
		);

		load_main_menu();

		return true;
	}

	void UISystem::init_game_ui()
	{
		UI& common_ui = m_game_core.get_core().get_ui();

		// FIXME: have a more robust way to connect UI to game functions!
		// Implement scripting?
		common_ui.register_selectable_callback("start",
			[this](std::string_view /*key*/)
			{
				start_game();
			}
		);

		common_ui.register_selectable_callback("quit",
			[this](std::string_view /*key*/)
			{
				m_game_core.request_shutdown();
			}
		);

		common_ui.register_selectable_callback("pause",
			[this](std::string_view /*key*/)
			{
				Model::GameModel& game_model = m_game_core.get_model();
				if (game_model.get_sim_state() == Model::GameModel::SimState::PLAYING)
				{
					game_model.set_paused(true);
				}
				else
				{
					game_model.set_paused(false);
				}
			}
		);

		common_ui.register_selectable_callback("return_to_main",
			[this](std::string_view /*key*/)
			{
				return_to_main_menu();
			}
		);
	}

	void UISystem::init_dev_gui()
	{
		// Set main window for dev GUI
		VadonApp::UI::Developer::GUISystem& dev_gui = m_game_core.get_engine_app().get_system<VadonApp::UI::Developer::GUISystem>();
		dev_gui.set_platform_window(m_game_core.get_platform_interface().get_main_window());
	}

	void UISystem::update()
	{
		m_main_window.update();
		if (m_dev_gui_enabled == true)
		{
			update_dev_gui();
		}

		if (m_deferred_init_queue.empty() == false)
		{
			for (Vadon::ECS::EntityHandle current_entity : m_deferred_init_queue)
			{
				init_entity(current_entity);
			}

			m_deferred_init_queue.clear();
		}

		CursorState cursor_state;

		VadonDemo::Platform::PlatformInterface& game_platform_interface = m_game_core.get_platform_interface();
		VadonApp::Platform::PlatformInterface& platform_interface = m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
		const VadonApp::Platform::MouseState mouse_state = platform_interface.get_mouse_state();

		const Vadon::Utilities::Vector2i window_size = platform_interface.get_window_size(game_platform_interface.get_main_window());

		cursor_state.position.x = mouse_state.position.x - (window_size.x * 0.5f);
		cursor_state.position.y = (window_size.y * 0.5f) - mouse_state.position.y;

		// TODO: could also handle platform events directly?
		VadonApp::Platform::InputSystem& input_system = m_game_core.get_engine_app().get_system<VadonApp::Platform::InputSystem>();
		const VadonApp::Platform::InputActionHandle ui_select_action = game_platform_interface.get_action(Platform::GameInputAction::UI_SELECT);
		const bool clicked = input_system.is_action_pressed(ui_select_action);
		if ((m_was_clicked == true) && (clicked == false))
		{
			cursor_state.clicked = true;
		}
		m_was_clicked = clicked;

		m_game_core.get_core().get_ui().update(m_game_core.get_ecs_world(), cursor_state);
	}

	void UISystem::update_dev_gui()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

		dev_gui.start_frame();

		// Draw console first
		VadonApp::UI::Console& app_console = engine_app.get_system<VadonApp::UI::UISystem>().get_console();
		app_console.render();

		for (const DevGUICallback& current_callback : m_dev_gui_callbacks)
		{
			current_callback(dev_gui);
		}

		dev_gui.end_frame();
	}

	void UISystem::init_entity(Vadon::ECS::EntityHandle entity)
	{
		m_game_core.get_core().get_ui().update_ui_element(m_game_core.get_ecs_world(), entity);
	}

	void UISystem::remove_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		VadonDemo::UI::Base* base_component = component_manager.get_component<VadonDemo::UI::Base>(entity);
		if (base_component != nullptr)
		{
			m_game_core.get_core().get_ui().remove_ui_element(ecs_world, entity);
		}
	}

	void UISystem::start_game()
	{
		VADON_ASSERT(m_main_menu_entity.is_valid() == true, "Main menu not loaded!");

		// Remove main menu
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		ecs_world.remove_entity(m_main_menu_entity);
		m_main_menu_entity.invalidate();

		// Load default level
		const VadonDemo::Core::CoreComponent& core_component = m_game_core.get_core_component();

		Model::GameModel& game_model = m_game_core.get_model();
		game_model.load_level(Model::GameModel::LevelConfiguration{ .scene_id = core_component.default_start_level });
	}

	void UISystem::return_to_main_menu()
	{
		Model::GameModel& game_model = m_game_core.get_model();
		game_model.quit_level();

		load_main_menu();
	}

	void UISystem::load_main_menu()
	{
		VADON_ASSERT(m_main_menu_entity.is_valid() == false, "Main menu not loaded!");
		const VadonDemo::Core::CoreComponent& core_component = m_game_core.get_core_component();

		Vadon::Scene::SceneSystem& scene_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		m_main_menu_entity = scene_system.instantiate_scene(core_component.main_menu, m_game_core.get_ecs_world());
	}
}