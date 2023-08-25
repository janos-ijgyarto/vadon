#ifndef VADONAPP_UTILITIES_EVENT_EVENT_HPP
#define VADONAPP_UTILITIES_EVENT_EVENT_HPP
#include <Vadon/Utilities/Event/Event.hpp>
namespace VadonApp::Utilities
{
	template<typename R, typename... Args>
	using EventHandler = Vadon::Utilities::EventHandler<R, Args...>;

	template<typename T>
	using EventDispatcher = Vadon::Utilities::EventDispatcher<T>;

	template<typename... Args>
	using EventDispatcherPool = Vadon::Utilities::EventDispatcherPool<Args...>;

	template<typename T, typename R>
	using Event = Vadon::Utilities::Event<T, R>;
}
#endif