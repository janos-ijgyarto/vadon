#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Private/UI/Developer/GUI.hpp>

#ifdef VADON_DEVELOPER_GUI_IMGUI
#include <VadonApp/Private/UI/Developer/ImGui/GUI.hpp>
#endif
#include <VadonApp/Private/UI/Developer/Dummy/GUI.hpp>

namespace VadonApp::Private::UI::Developer
{
	GUISystem::Implementation GUISystem::get_gui(Core::Application& application)
	{
#ifdef VADON_DEVELOPER_GUI_IMGUI
		return std::make_unique<ImGUI::GUISystem>(application);
#else
		return get_dummy_gui(application);
#endif
	}

	GUISystem::Implementation GUISystem::get_dummy_gui(Core::Application& application)
	{
		return std::make_unique<Dummy::GUISystem>(application);
	}

	GUISystem::GUISystem(VadonApp::Core::Application& application)
		: VadonApp::UI::Developer::GUISystem(application)
	{
	}
}