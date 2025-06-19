#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/UI/UISystem.hpp>

#include <VadonApp/Private/Core/Application.hpp>

namespace VadonApp::Private::UI
{
	UISystem::~UISystem() = default;

	bool UISystem::initialize()
	{
		// TODO: use config!
		if (!m_dev_gui->initialize())
		{
			return false;
		}

		return true;
	}

	void UISystem::shutdown()
	{
		// TODO!!!
	}

	void UISystem::update()
	{
		// TODO!!!
	}

	UISystem::UISystem(Core::Application& application)
		: VadonApp::UI::UISystem(application)
	{
		m_dev_gui = Developer::GUISystem::get_gui(application);
	}
}
