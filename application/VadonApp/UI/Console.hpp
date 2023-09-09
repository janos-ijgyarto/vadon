#ifndef VADONAPP_UI_CONSOLE_HPP
#define VADONAPP_UI_CONSOLE_HPP
#include <Vadon/Core/Logger.hpp>
#include <Vadon/Utilities/Event/Event.hpp>
namespace VadonApp::UI
{
	struct ConsoleCommandEvent : public Vadon::Utilities::Event<const ConsoleCommandEvent&, bool>
	{
		std::string text;
	};

	// Console also implements the engine logger interface
	class Console : public Vadon::Core::Logger
	{
	public:
		virtual void show() = 0;
		virtual bool is_visible() const = 0;
		virtual void render() = 0;

		void register_event_handler(ConsoleCommandEvent::Handler&& handler)
		{
			m_event_dispatcher_pool.get_dispatcher<ConsoleCommandEvent::Handler>().register_handler(std::move(handler));
		}
	protected:
		using ConsoleEventDispatcherPool = Vadon::Utilities::EventDispatcherPool<ConsoleCommandEvent::Handler>;

		ConsoleEventDispatcherPool m_event_dispatcher_pool;
	};
}
#endif