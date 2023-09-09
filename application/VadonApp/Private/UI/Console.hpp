#ifndef VADONAPP_PRIVATE_UI_CONSOLE_HPP
#define VADONAPP_PRIVATE_UI_CONSOLE_HPP
#include <VadonApp/UI/Console.hpp>
#include <VadonApp/UI/Developer/GUIElements.hpp>

#include <shared_mutex>

namespace VadonApp::Private::Core
{
	class Application;
}
namespace VadonApp::Private::UI
{
	class Console : public VadonApp::UI::Console
	{
	public:
		void log(std::string_view message) override;
		void warning(std::string_view message) override;
		void error(std::string_view message) override;

		void show() override;
		bool is_visible() const override { return m_window.open; }
		void render() override;
	private:
		Console(Core::Application& application);

		bool initialize();
		void append(std::string_view text);

		Core::Application& m_application;

		std::vector<std::string> m_history;
		std::vector<std::string> m_command_history;

		std::shared_mutex m_history_mutex;

		VadonApp::UI::Developer::Window m_window;
		VadonApp::UI::Developer::InputText m_input;

		friend class UISystem;
	};
}
#endif