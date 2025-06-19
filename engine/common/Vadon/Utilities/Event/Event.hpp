#ifndef VADON_UTILITIES_EVENT_EVENT_HPP
#define VADON_UTILITIES_EVENT_EVENT_HPP
#include <Vadon/Utilities/Event/EventHandler.hpp>
#include <Vadon/Utilities/Event/EventDispatcher.hpp>
namespace Vadon::Utilities
{
	// Using CRTP to make it less cumbersome to declare objects related to event handling
	template<typename T, typename R>
	struct Event
	{
		using Handler = EventHandler<R, T>;
		using Dispatcher = EventDispatcher<Handler>;
	};
}
#endif