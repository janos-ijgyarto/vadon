#ifndef VADONAPP_UI_UIMODULE_HPP
#define VADONAPP_UI_UIMODULE_HPP
#include <VadonApp/Core/System.hpp>
#include <VadonApp/Core/SystemModule.hpp>
namespace VadonApp::UI::Developer
{
	class GUISystem;
}
namespace VadonApp::UI
{
	class UIModule : public Core::SystemModule<Core::SystemModuleList, UIModule, class UISystem, class Developer::GUISystem>
	{
	};

	template<typename SysImpl>
	using UISystemBase = Core::System<UIModule, SysImpl>;
}
#endif