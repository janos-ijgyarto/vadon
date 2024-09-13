#ifndef VADONAPP_UI_UISYSTEM_HPP
#define VADONAPP_UI_UISYSTEM_HPP
#include <VadonApp/UI/Module.hpp>
namespace VadonApp::UI::Developer
{
	class GUISystem;
}
namespace VadonApp::UI
{
	class Console;

	class UISystem : public UISystemBase<UISystem>
	{
	public:
		virtual Console& get_console() = 0;
	protected:
		UISystem(Core::Application& application) : System(application) {}
	};
}
#endif