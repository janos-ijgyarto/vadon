#include <VadonEditor/Platform/PlatformInterface.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

#include <VadonApp/Platform/Input/InputSystem.hpp>

namespace VadonEditor::Platform
{
	struct PlatformInterface::Internal
	{
		std::vector<PlatformEventCallback> m_event_callbacks;

		bool initialize()
		{
			return true;
		}

		void update(Core::Editor& editor)
		{
			VadonApp::Platform::PlatformInterface& platform_interface = editor.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			const VadonApp::Platform::PlatformEventList platform_events = platform_interface.read_events();

			editor.get_engine_app().get_system<VadonApp::Platform::InputSystem>().dispatch_platform_events(platform_events);

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

	PlatformInterface::PlatformInterface(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>())
	{

	}

	bool PlatformInterface::initialize()
	{
		return m_internal->initialize();
	}

	void PlatformInterface::update()
	{
		m_internal->update(m_editor);
	}
}