#ifndef VADONEDITOR_UI_UISYSTEM_HPP
#define VADONEDITOR_UI_UISYSTEM_HPP
class QByteArray;
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class LauncherDialog;
	class MainWindow;

	class RenderWidget;

	class UISystem
	{
	public:
	private:
		UISystem(Core::Application& application);

		bool initialize();
		void shutdown();

		void launcher_accepted();

		void show_main_window();
		void request_close();

		void received_message(const QByteArray& data);

		void run_simulator();
		void stop_simulator();

		Core::Application& m_application;

		LauncherDialog* m_launcher_dialog;
		MainWindow* m_main_window;

		friend Core::Application;
	};
}
#endif