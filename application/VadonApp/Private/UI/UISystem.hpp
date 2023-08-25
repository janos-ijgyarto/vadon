#ifndef VADONAPP_PRIVATE_UI_UISYSTEM_HPP
#define VADONAPP_PRIVATE_UI_UISYSTEM_HPP
#include <VadonApp/UI/UISystem.hpp>

#include <VadonApp/Private/UI/Console.hpp>
#include <VadonApp/Private/UI/Developer/GUI.hpp>

namespace VadonApp::Private::Core
{
	class Application;
}

namespace VadonApp::Private::UI
{
	class UISystem : public VadonApp::UI::UISystem
	{
	public:
		~UISystem();

		bool initialize();
		void shutdown();

		void update();

		VadonApp::UI::Console& get_console() override { return m_console; }
	private:
		// FIXME: this does mean the app may have to know about every 
		UISystem(Core::Application& application);

		Console m_console;
		Developer::GUISystem::Implementation m_dev_gui;

		friend class Core::Application;
	};
}
#endif