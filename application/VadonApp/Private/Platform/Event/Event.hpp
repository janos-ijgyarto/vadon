#ifndef VADONAPP_PRIVATE_PLATFORM_EVENT_EVENT_HPP
#define VADONAPP_PRIVATE_PLATFORM_EVENT_EVENT_HPP
#include <VadonApp/Platform/Event/Event.hpp>
namespace VadonApp::Private::Platform
{
	// FIXME: might be better to use std::variant?
	class EventWrapperBase
	{
	public:
		virtual ~EventWrapperBase() {}
		virtual void dispatch(VadonApp::Platform::PlatformEventDispatcherPool& pool) = 0;
	};

	using EventWrapperPointer = std::unique_ptr<EventWrapperBase>;

	template<typename T>
	class EventWrapper : public EventWrapperBase
	{
	public:
		EventWrapper(const T& event) : m_event(event) {}

		void dispatch(VadonApp::Platform::PlatformEventDispatcherPool& pool) override
		{
			using Dispatcher = typename T::Dispatcher;
			using Handler = typename T::Handler;

			Dispatcher& event_dispatcher = pool.get_dispatcher<Handler>();
			for (const Handler& current_handler : event_dispatcher.get_handlers())
			{
				if (current_handler(m_event))
				{
					break;
				}
			}
		}
	private:
		T m_event;
	};

	using WindowEventWrapper = EventWrapper<VadonApp::Platform::WindowEvent>;
	using MouseMotionEventWrapper = EventWrapper<VadonApp::Platform::MouseMotionEvent>;
	using MouseButtonEventWrapper = EventWrapper<VadonApp::Platform::MouseButtonEvent>;
	using KeyboardEventWrapper = EventWrapper<VadonApp::Platform::KeyboardEvent>;
	using TextInputEventWrapper = EventWrapper<VadonApp::Platform::TextInputEvent>;
	using QuitEventWrapper = EventWrapper<VadonApp::Platform::QuitEvent>;
}
#endif