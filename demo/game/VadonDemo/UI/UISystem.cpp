#include <VadonDemo/UI/UISystem.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <Vadon/Utilities/Data/Visitor.hpp>

#include <format>

namespace VadonDemo::UI
{
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

		return true;
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
}