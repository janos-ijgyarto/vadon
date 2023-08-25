#ifndef VADON_UTILITIES_EVENT_EVENTHANDLER_HPP
#define VADON_UTILITIES_EVENT_EVENTHANDLER_HPP
#include <functional>
namespace Vadon::Utilities
{
	// Callback function to which events will be dispatched
	template<typename R, typename...Args>
	using EventHandler = std::function<R(Args...)>;
}
#endif