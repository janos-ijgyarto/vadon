#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Platform/PlatformInterface.hpp>

#include <VadonEditor/UI/Developer/GUI.hpp>

#include <VadonApp/Core/Application.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>

#include <Vadon/Utilities/Data/VariantUtilities.hpp>

#include <format>
#include <unordered_map>

namespace VadonEditor::UI
{
	struct UISystem::Internal
	{
		Core::Editor& m_editor;

		std::unordered_map<std::string, ConsoleCallback> m_console_commands;
		std::vector<UICallback> m_ui_elements;

		Internal(Core::Editor& editor)
			: m_editor(editor)
		{}

		void register_console_command(std::string_view command, const ConsoleCallback& callback)
		{
			m_console_commands.emplace(command, callback);
		}

		bool initialize()
		{
			Platform::PlatformInterface& platform_interface = m_editor.get_system<Platform::PlatformInterface>();
			platform_interface.register_event_callback(
				[this](const VadonApp::Platform::PlatformEventList& platform_events)
				{
					auto event_handler = Vadon::Utilities::VisitorOverloadList{
						[this](const VadonApp::Platform::KeyboardEvent& keyboard_event)
						{
							if (keyboard_event.key == VadonApp::Platform::KeyCode::BACKQUOTE)
							{
								VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
								VadonApp::UI::UISystem& ui_system = engine_app.get_system<VadonApp::UI::UISystem>();
								VadonApp::UI::Console& app_console = ui_system.get_console();
								app_console.show();
							}
						},
						[](auto) {}
					};

					for (const VadonApp::Platform::PlatformEvent& current_event : platform_events)
					{
						std::visit(event_handler, current_event);
					}

					// Dispatch events to dev GUI
					VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
					VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
					dev_gui.dispatch_platform_events(platform_events);
				}
			);

			// App console
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			{
				// Handle console events
				VadonApp::UI::UISystem& ui_system = engine_app.get_system<VadonApp::UI::UISystem>();
				VadonApp::UI::Console& app_console = ui_system.get_console();

				app_console.register_event_handler(
					[this, &app_console](const VadonApp::UI::ConsoleCommandEvent& command_event)
					{
						if (parse_console_command(command_event.text) == true)
						{
							return true;
						}

						app_console.error(std::format("Command not recognized: \"{}\"\n", command_event.text));
						return true;
					}
				);
			}

			return true;
		}

		void update()
		{
			update_dev_gui();
		}

		void update_dev_gui()
		{
			// FIXME: should we have some way to early out?
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			VadonApp::UI::UISystem& ui_system = engine_app.get_system<VadonApp::UI::UISystem>();
			VadonApp::UI::Console& app_console = ui_system.get_console();

			VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();

			dev_gui.start_frame();

			// Draw console
			app_console.render();

			// Run the registered UI elements
			for (const UICallback& current_ui_element : m_ui_elements)
			{
				current_ui_element(m_editor);
			}

			dev_gui.end_frame();
		}

		bool parse_console_command(std::string_view command)
		{
			auto command_it = m_console_commands.find(std::string(command));
			if (command_it == m_console_commands.end())
			{
				return false;
			}

			// Command found, run it
			command_it->second();
			return true;
		}
	};

	UISystem::~UISystem() = default;

	void UISystem::register_console_command(std::string_view command, const ConsoleCallback& callback)
	{
		m_internal->register_console_command(command, callback);
	}

	void UISystem::register_ui_element(const UICallback& callback)
	{
		m_internal->m_ui_elements.push_back(callback);
	}

	UISystem::UISystem(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{
	}

	bool UISystem::initialize()
	{
		return m_internal->initialize();
	}

	void UISystem::update()
	{
		return m_internal->update();
	}
}