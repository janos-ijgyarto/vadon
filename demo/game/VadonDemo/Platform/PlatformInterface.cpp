#include <VadonDemo/Platform/PlatformInterface.hpp>

#include <VadonDemo/Core/GameCore.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

namespace VadonDemo::Platform
{
	struct PlatformInterface::Internal
	{
		Core::GameCore& m_game_core;

		std::vector<PlatformEventCallback> m_event_callbacks;

		Internal(Core::GameCore& game_core)
			: m_game_core(game_core)
		{

		}

		bool initialize()
		{
			// TODO!!!
			return true;
		}

		void update()
		{
			VadonApp::Platform::PlatformInterface& platform_interface = m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::PlatformEventList platform_events = platform_interface.read_events();

			if (platform_events.empty())
			{
				return;
			}

			// Propagate to all callbacks (it's their responsibility to manage data races, if needed)
			for (const PlatformEventCallback& current_callback : m_event_callbacks)
			{
				current_callback(platform_events);
			}
		}
	};

	PlatformInterface::~PlatformInterface() = default;

	void PlatformInterface::register_event_callback(const PlatformEventCallback& callback)
	{
		m_internal->m_event_callbacks.push_back(callback);
	}

	void PlatformInterface::move_window(Vadon::Utilities::Vector2i position)
	{
		VadonApp::Platform::PlatformInterface& platform_interface = m_internal->m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
		platform_interface.move_window(position);
	}

	void PlatformInterface::resize_window(Vadon::Utilities::Vector2i size)
	{
		VadonApp::Platform::PlatformInterface& platform_interface = m_internal->m_game_core.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
		platform_interface.resize_window(size);
	}

	PlatformInterface::PlatformInterface(Core::GameCore& game_core)
		: m_internal(std::make_unique<Internal>(game_core))
	{

	}

	bool PlatformInterface::initialize()
	{
		return m_internal->initialize();
	}

	void PlatformInterface::update()
	{
		m_internal->update();
	}
}