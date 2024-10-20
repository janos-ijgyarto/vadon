#include <VadonEditor/Platform/PlatformInterface.hpp>

#include <VadonApp/Core/Application.hpp>
#include <VadonApp/Platform/PlatformInterface.hpp>

namespace VadonEditor::Platform
{
	struct PlatformInterface::Internal
	{
		bool initialize()
		{
			return true;
		}

		void update(Core::Editor& editor)
		{
			VadonApp::Platform::PlatformInterface& platform_interface = editor.get_engine_app().get_system<VadonApp::Platform::PlatformInterface>();
			platform_interface.dispatch_events();
		}
	};

	PlatformInterface::~PlatformInterface() = default;

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