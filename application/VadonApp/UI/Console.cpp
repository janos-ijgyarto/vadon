#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/UI/Console.hpp>

#include <format>

namespace VadonApp::UI
{
	void Console::add_text(std::string_view text)
	{
		size_t prev_length = m_log.buffer.size();
		m_log.buffer += text;
		for (size_t index = prev_length; index < m_log.buffer.size(); ++index)
		{
			if (m_log.buffer[index] == '\n')
			{
				m_log.line_offsets.push_back(index + 1);
			}
		}
	}

	void Console::submit_command(std::string_view command)
	{
		// First add all commands to log and history
		size_t prev_command_count = m_command_history.size();

		size_t current_command_start = 0;
		for (size_t command_char_index = 0; command_char_index < command.size(); ++command_char_index)
		{
			if (m_log.buffer[command_char_index] == '\n')
			{
				internal_submit_command(command, current_command_start, command_char_index - current_command_start);
				current_command_start = command_char_index + 1;
			}
		}

		internal_submit_command(command, current_command_start, command.size() - current_command_start);

		// Go over history, dispatch commands to handlers
		const size_t line_count = m_log.line_offsets.size();
		for (size_t command_index = prev_command_count; command_index < m_command_history.size(); ++command_index)
		{
			size_t command_line_index = m_command_history[command_index];

			const char* command_start = m_log.buffer.data() + m_log.line_offsets[command_line_index];
			const char* command_end = ((command_line_index + 1) < line_count) ? (m_log.buffer.data() + m_log.line_offsets[command_line_index + 1] - 1) : std::to_address(m_log.buffer.end());

			// Dispatch to handlers
			const ConsoleCommandEvent command_event{ .text = std::string(command_start, command_end)};
			for (const ConsoleCommandEvent::Handler& current_handler : m_event_dispatcher_pool.get_dispatcher<ConsoleCommandEvent::Handler>().get_handlers())
			{
				current_handler(command_event);
			}
		}
	}

	void Console::clear()
	{
		m_log.buffer.clear();
		m_log.line_offsets.clear();

		m_log.line_offsets.push_back(0);

		m_command_history.clear();
	}

	void Console::internal_submit_command(std::string_view command, size_t offset, size_t length)
	{
		if (length == 0)
		{
			return;
		}

		const std::string_view command_substr = command.substr(offset, length);

		// Add to history (most recent command will start at the last line offset)
		m_command_history.push_back(m_log.line_offsets.size() - 1);

		// Add to log
		add_text(std::format("{}\n", command_substr));
	}
}