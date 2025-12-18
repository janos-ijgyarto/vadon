#include <VadonEditor/UI/UISystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Platform/PlatformInterface.hpp>

#include <VadonEditor/UI/Developer/GUI.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/UI/UISystem.hpp>
#include <VadonApp/UI/Console.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Utilities/Data/Visitor.hpp>

#include <format>
#include <shared_mutex>
#include <unordered_map>

namespace
{
	// TODO: backport to application library?
	class EditorLogger : public Vadon::Core::DefaultLogger
	{
	public:
		EditorLogger(VadonEditor::Core::Editor& editor)
			: m_editor(editor)
		{ 
			m_console_window.title = "Editor Console";
			m_console_window.open = true;

			m_log_child_window.string_id = "##log";
			m_log_child_window.flags |= VadonApp::UI::Developer::WindowFlags::HORIZONTAL_SCROLLBAR;

			m_input.label = "Input";

			Vadon::Core::EngineEnvironment::set_logger(this);
		}

		~EditorLogger()
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

		void draw_console(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			if (dev_gui.begin_window(m_console_window) == true)
			{
				VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
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
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			VadonApp::UI::Console& console = engine_app.get_system<VadonApp::UI::UISystem>().get_console();

			std::unique_lock lock(m_mutex);
			console.add_text(message);
		}

		VadonEditor::Core::Editor& m_editor;
		std::shared_mutex m_mutex;

		VadonApp::UI::Developer::Window m_console_window;
		VadonApp::UI::Developer::ChildWindow m_log_child_window;
		VadonApp::UI::Developer::InputText m_input;
	};
}

namespace VadonEditor::UI
{
	struct UISystem::Internal
	{
		Core::Editor& m_editor;

		std::unordered_map<std::string, ConsoleCallback> m_console_commands;
		std::vector<UICallback> m_ui_elements;

		EditorLogger m_logger;

		Internal(Core::Editor& editor)
			: m_editor(editor)
			, m_logger(editor)
		{
		}

		void register_console_command(std::string_view command, const ConsoleCallback& callback)
		{
			m_console_commands.emplace(command, callback);
		}

		bool initialize()
		{
			VadonApp::Core::Application& engine_app = m_editor.get_engine_app();
			// Set main window for dev GUI
			{
				VadonApp::UI::Developer::GUISystem& dev_gui = engine_app.get_system<VadonApp::UI::Developer::GUISystem>();
				dev_gui.set_platform_window(m_editor.get_system<Platform::PlatformInterface>().get_main_window());
			}

			// App console
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

						Vadon::Core::Logger::log_error(std::format("Command not recognized: \"{}\"\n", command_event.text));
						return true;
					}
				);
			}

			return true;
		}

		void draw_ui(VadonApp::UI::Developer::GUISystem& dev_gui)
		{
			m_logger.draw_console(dev_gui);

			// Run the registered UI elements
			for (const UICallback& current_ui_element : m_ui_elements)
			{
				current_ui_element(m_editor);
			}
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

	void UISystem::draw_ui(VadonApp::UI::Developer::GUISystem& dev_gui)
	{
		m_internal->draw_ui(dev_gui);
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
}