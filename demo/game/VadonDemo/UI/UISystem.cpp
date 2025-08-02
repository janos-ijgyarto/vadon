#include <VadonDemo/UI/UISystem.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Model/GameModel.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/Render/RenderSystem.hpp>

#include <VadonDemo/UI/Component.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>
#include <VadonApp/Platform/Input/InputSystem.hpp>

#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Utilities/Data/Visitor.hpp>

#include <format>
#include <shared_mutex>

namespace VadonDemo::UI
{
	// TODO: backport to application library?
	class UISystem::GameLogger : public Vadon::Core::DefaultLogger
	{
	public:
		GameLogger(VadonDemo::Core::GameCore& game_core)
			: m_game_core(game_core)
		{
			m_console_window.title = "Game Console";
			m_console_window.open = true;
			m_console_window.flags = VadonApp::UI::Developer::WindowFlags::ENABLE_CLOSE;

			m_log_child_window.id = "##log";
			m_log_child_window.flags |= VadonApp::UI::Developer::WindowFlags::HORIZONTAL_SCROLLBAR;

			m_input.label = "Input";

			Vadon::Core::EngineEnvironment::set_logger(this);
		}

		~GameLogger()
		{
			Vadon::Core::EngineEnvironment::set_logger(nullptr);
		}

		void log_message(std::string_view message) override
		{
			Vadon::Core::DefaultLogger::log_message(message);
			append_console(message);
		}

		void log_warning(std::string_view message) override
		{
			Vadon::Core::DefaultLogger::log_warning(message);
			append_console(message);
		}

		void log_error(std::string_view message) override
		{
			Vadon::Core::DefaultLogger::log_error(message);
			append_console(message);
		}

		void show_console()
		{
			m_console_window.open = true;
		}

		void draw_console(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			if (dev_gui.begin_window(m_console_window) == true)
			{
				VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
				VadonApp::UI::Console& console = engine_app.get_system<VadonApp::UI::UISystem>().get_console();

				m_log_child_window.size = dev_gui.get_available_content_region();

				// TODO: use separator, ensure enough space is left for it!
				const VadonApp::UI::Developer::GUIStyle gui_style = dev_gui.get_style();
				m_log_child_window.size.y -= dev_gui.calculate_text_size(m_input.label).y + gui_style.frame_padding.y * 2 + 5.0f;

				if (dev_gui.begin_child_window(m_log_child_window) == true)
				{
					std::shared_lock lock(m_mutex);

					const VadonApp::UI::Console::LineBuffer& console_log = console.get_log();
					dev_gui.draw_clipped_text_list(VadonApp::UI::Developer::TextBuffer{ .buffer = console_log.buffer, .line_offsets = console_log.line_offsets });
				}
				dev_gui.end_child_window();

				if (dev_gui.draw_input_text(m_input))
				{
					console.submit_command(m_input.input);
					m_input.input.clear();
				}
			}
			dev_gui.end_window();
		}
	private:
		void append_console(std::string_view message)
		{
			VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
			VadonApp::UI::Console& console = engine_app.get_system<VadonApp::UI::UISystem>().get_console();

			std::unique_lock lock(m_mutex);
			console.add_text(message);
		}

		VadonDemo::Core::GameCore& m_game_core;
		std::shared_mutex m_mutex;

		VadonApp::UI::Developer::Window m_console_window;
		VadonApp::UI::Developer::ChildWindow m_log_child_window;
		VadonApp::UI::Developer::InputText m_input;
	};

	UISystem::~UISystem() = default;

	void UISystem::register_dev_gui_callback(const DevGUICallback& callback)
	{
		m_dev_gui_callbacks.push_back(callback);
	}

	void UISystem::register_console_command(std::string_view command_name, const ConsoleCommandCallback& callback)
	{
		m_console_commands[std::string(command_name)] = callback;
	}

	void UISystem::show_console()
	{
		m_logger->show_console();
	}

	UISystem::UISystem(Core::GameCore& core)
		: m_game_core(core)
		, m_main_window(core)
		, m_logger(std::make_unique<GameLogger>(core))
		, m_entities_dirty(false)
	{
		m_dev_gui_window.title = "UI System";
	}

	bool UISystem::initialize()
	{
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
						Vadon::Core::Logger::log_error(std::format("Command not recognized: \"{}\"\n", command_name));
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

		m_game_core.get_core().add_entity_event_callback(
			[this](Vadon::ECS::World& /*ecs_world*/, const VadonDemo::Core::EntityEvent& event)
			{
				switch (event.type)
				{
				case VadonDemo::Core::EntityEventType::ADDED:
					init_entity(event.entity);
					break;
				case VadonDemo::Core::EntityEventType::REMOVED:
					remove_entity(event.entity);
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

		register_dev_gui_callback(
			[this](VadonApp::UI::Developer::GUISystem& dev_gui)
			{
				if (dev_gui.begin_window(m_dev_gui_window))
				{
					dev_gui.add_text(std::format("Mouse position: {} {}", m_mouse_pos.x, m_mouse_pos.y));
				}
				dev_gui.end_window();
			}
		);
	}

	void UISystem::update()
	{
		// Register callback in platform interface
		VadonApp::Platform::PlatformInterface& platform_interface = m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			
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

		for (const VadonApp::Platform::PlatformEvent& current_event : platform_interface.poll_events())
		{
			std::visit(platform_event_visitor, current_event);
		}

		m_main_window.update();
		if (m_dev_gui_enabled == true)
		{
			update_dev_gui();
		}

		update_dirty_entities();

		const VadonApp::Platform::MouseState mouse_state = platform_interface.get_mouse_state();

		const Vadon::Math::Vector2i mouse_position = m_game_core.get_render_system().map_to_game_viewport(mouse_state.position);
		const Core::GlobalConfiguration& global_config = m_game_core.get_core().get_global_config();

		CursorState cursor_state;
		cursor_state.position.x = mouse_position.x - (global_config.viewport_size.x * 0.5f);
		cursor_state.position.y = (global_config.viewport_size.y * 0.5f) - mouse_position.y;

		// TODO: could also handle platform events directly?
		VadonDemo::Platform::PlatformInterface& game_platform_interface = m_game_core.get_platform_interface();
		const VadonApp::Platform::InputActionHandle ui_select_action = game_platform_interface.get_action(Platform::GameInputAction::UI_SELECT);

		VadonApp::Platform::InputSystem& input_system = m_game_core.get_engine_app().get_system<VadonApp::Platform::InputSystem>();
		const bool clicked = input_system.is_action_pressed(ui_select_action);
		if ((m_was_clicked == true) && (clicked == false))
		{
			cursor_state.clicked = true;
		}
		m_was_clicked = clicked;

		m_game_core.get_core().get_ui().update(m_game_core.get_ecs_world(), cursor_state);

		m_mouse_pos = cursor_state.position;
	}

	void UISystem::update_dirty_entities()
	{
		if (m_entities_dirty == false)
		{
			return;
		}

		m_entities_dirty = false;

		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		auto base_ui_query = ecs_world.get_component_manager().run_component_query<Base&>();

		UI& common_ui = m_game_core.get_core().get_ui();

		for (auto base_it = base_ui_query.get_iterator(); base_it.is_valid() == true; base_it.next())
		{
			auto base_tuple = base_it.get_tuple();
			Base& current_base_component = std::get<Base&>(base_tuple);

			if (current_base_component.dirty == false)
			{
				continue;
			}

			// Update draw data (if applicable)
			common_ui.update_ui_element(ecs_world, base_it.get_entity());

			// Unset the flag
			current_base_component.dirty = false;
		}
	}

	void UISystem::update_dev_gui()
	{
		VadonApp::Core::Application& engine_app = m_game_core.get_engine_app();
		VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

		dev_gui.start_frame();

		// Draw console first
		m_logger->draw_console(dev_gui);

		for (const DevGUICallback& current_callback : m_dev_gui_callbacks)
		{
			current_callback(dev_gui);
		}

		dev_gui.end_frame();
	}

	void UISystem::init_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();

		// Make sure we at least have a base UI component!
		Base* base_component = ecs_world.get_component_manager().get_component<Base>(entity);
		if (base_component == nullptr)
		{
			return;
		}

		base_component->dirty = true;
		m_entities_dirty = true;
	}

	void UISystem::remove_entity(Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		VadonDemo::UI::Base* base_component = component_manager.get_component<VadonDemo::UI::Base>(entity);
		if (base_component == nullptr)
		{
			return;
		}

		m_game_core.get_core().get_ui().remove_ui_element(ecs_world, entity);
	}

	void UISystem::start_game()
	{
		VADON_ASSERT(m_main_menu_entity.is_valid() == true, "Main menu not loaded!");

		// Remove main menu
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();

		// Dispatch events (to ensure we clean up used resources)
		m_game_core.get_core().entity_removed(ecs_world, m_main_menu_entity);

		ecs_world.remove_entity(m_main_menu_entity);
		m_main_menu_entity.invalidate();

		// Load default level
		const Core::GlobalConfiguration& global_config = m_game_core.get_core().get_global_config();

		Model::GameModel& game_model = m_game_core.get_model();
		if (game_model.load_level(Model::GameModel::LevelConfiguration{ .scene_id = global_config.default_start_level }) == false)
		{
			// In case of failure, go back to main menu
			load_main_menu();
		}
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
		const Core::GlobalConfiguration& global_config = m_game_core.get_core().get_global_config();
		VADON_ASSERT(global_config.main_menu_scene.is_valid() == true, "Need a valid main menu scene!");

		Vadon::Scene::SceneSystem& scene_system = m_game_core.get_engine_app().get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		Vadon::ECS::World& ecs_world = m_game_core.get_ecs_world();
		Vadon::Scene::SceneHandle main_menu_scene = scene_system.load_scene(global_config.main_menu_scene);
		m_main_menu_entity = scene_system.instantiate_scene(main_menu_scene, ecs_world);

		// Dispatch events
		m_game_core.get_core().entity_added(ecs_world, m_main_menu_entity);
	}
}