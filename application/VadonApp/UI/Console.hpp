#ifndef VADONAPP_UI_CONSOLE_HPP
#define VADONAPP_UI_CONSOLE_HPP
#include <VadonApp/VadonApp.hpp>
#include <Vadon/Utilities/Event/Event.hpp>
namespace VadonApp::UI
{
	struct ConsoleCommandEvent : public Vadon::Utilities::Event<const ConsoleCommandEvent&, bool>
	{
		std::string text;
	};

	class Console
	{
	public:
		struct LineBuffer
		{
			std::string buffer;
			std::vector<size_t> line_offsets;
		};

		Console()
		{
			clear();
		}

		VADONAPP_API void add_text(std::string_view text);
		VADONAPP_API void submit_command(std::string_view command);

		void clear();

		void register_event_handler(ConsoleCommandEvent::Handler&& handler)
		{
			m_event_dispatcher_pool.get_dispatcher<ConsoleCommandEvent::Handler>().register_handler(std::move(handler));
		}

		const LineBuffer& get_log() const { return m_log; }
		const std::vector<size_t>& get_command_history() const { return m_command_history; }
	protected:
		using ConsoleEventDispatcherPool = Vadon::Utilities::EventDispatcherPool<ConsoleCommandEvent::Handler>;

		void internal_submit_command(std::string_view command, size_t offset, size_t length);

		LineBuffer m_log;
		std::vector<size_t> m_command_history;

		ConsoleEventDispatcherPool m_event_dispatcher_pool;
	};
}
#endif