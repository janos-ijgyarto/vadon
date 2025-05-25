#ifndef VADON_UTILITIES_EVENT_EVENTDISPATCHER_HPP
#define VADON_UTILITIES_EVENT_EVENTDISPATCHER_HPP
#include <vector>
#include <tuple>
namespace Vadon::Utilities
{
	// FIXME: some way to make it explicit that we expect an EventHandler here?
	template<typename T>
	class EventDispatcher
	{
	public:
		using EventHandler = T;
		using EventHandlerVector = std::vector<EventHandler>;

		// FIXME: enforce that 
		void register_handler(EventHandler&& handler)
		{
			m_handlers.emplace_back(handler);
		}

		// FIXME: currently we just delegate 
		const EventHandlerVector& get_handlers() const { return m_handlers; }
	private:
		std::vector<EventHandler> m_handlers;
	};

	// FIXME: use something other than a tuple?
	template<typename... Args>
	class EventDispatcherPool
	{
	public:
		template<typename T>
		EventDispatcher<T>& get_dispatcher() { return std::get<EventDispatcher<T>>(m_dispatchers); }
	private:
		std::tuple<EventDispatcher<Args>...> m_dispatchers;
	};
}
#endif