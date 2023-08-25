#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/UI/Console.hpp>

#include <VadonApp/Private/Core/Application.hpp>
#include <VadonApp/UI/Developer/GUI.hpp>

#include <iostream>
#include <syncstream>
#include <format>

namespace VadonApp::Private::UI
{
	void Console::log(std::string_view message)
	{
		std::osyncstream cout_sync(std::cout);
		cout_sync << message;

		append(message);
	}

	void Console::warning(std::string_view message)
	{
		const std::string warning_message = std::format("WARNING: {}", message);

		std::osyncstream cout_sync(std::cout);
		cout_sync << warning_message;

		append(warning_message);
	}

	void Console::error(std::string_view message)
	{
		const std::string error_message = std::format("ERROR: {}", message);

		std::osyncstream cerr_sync(std::cerr);
		cerr_sync << error_message;

		append(error_message);
	}

	void Console::show()
	{
		m_window.open = true;
	}

	void Console::render()
	{
		if (!m_window.open)
		{
			return;
		}

		VadonApp::UI::Developer::GUISystem& dev_gui = m_application.get_system<VadonApp::UI::Developer::GUISystem>();

		if (dev_gui.begin_window(m_window))
		{
			// Use lock to make sure we don't read while the history is being updated
			// FIXME: some way to make this lock-free?
			{
				std::shared_lock read_lock(m_history_mutex);
				for (const std::string& current_line : m_history)
				{
					dev_gui.add_text_unformatted(current_line);
				}
			}

			if (dev_gui.draw_input_text(m_input))
			{
				m_command_history.push_back(m_input.input);
				append(m_input.input + '\n');

				// Dispatch console event
				VadonApp::UI::ConsoleCommandEvent console_command_event{ .text = m_input.input };
				VadonApp::UI::ConsoleCommandEvent::Dispatcher& console_command_dispatcher = m_event_dispatcher_pool.get_dispatcher<VadonApp::UI::ConsoleCommandEvent::Handler>();
				for (const VadonApp::UI::ConsoleCommandEvent::Handler& current_console_handler : console_command_dispatcher.get_handlers())
				{
					if (current_console_handler(console_command_event))
					{
						break;
					}
				}
				m_input.input.clear();
			}
		}
		dev_gui.end_window();
	}

	Console::Console(Core::Application& application)
		: m_application(application)
	{
		// Add the first line
		m_history.emplace_back();
	}

	bool Console::initialize()
	{
		// Set up console window
		m_window.title = "Console";
		m_window.flags = VadonApp::UI::Developer::WindowFlags::ENABLE_CLOSE;

		m_input.label = "Input";

		return true;
	}

	void Console::append(std::string_view text)
	{
		// Go through the text, append a string for each line
		std::string::size_type current_offset = 0;
		std::string::size_type prev_offset = 0;

		constexpr std::string_view delimiter = "\n";

		// Push into local vector so we can add to the history vector in one go
		std::vector<std::string> history_update;
		history_update.emplace_back();

		while ((current_offset = text.find(delimiter, prev_offset)) != std::string::npos)
		{
			history_update.back() += text.substr(prev_offset, current_offset - prev_offset);
			history_update.emplace_back();

			prev_offset = current_offset + delimiter.size();
		}

		// Add what's left at the end
		history_update.back() += text.substr(prev_offset);

		// Use a write lock to update the history
		// FIXME: some way to make this lock-free?
		{
			std::unique_lock write_lock(m_history_mutex);

			m_history.back() += history_update.front();
			m_history.insert(m_history.end(), history_update.begin() + 1, history_update.end());
		}
	}
}