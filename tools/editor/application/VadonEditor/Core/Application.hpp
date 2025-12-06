#ifndef VADONEDITOR_CORE_APPLICATION_HPP
#define VADONEDITOR_CORE_APPLICATION_HPP
#include <memory>
namespace VadonEditor::Network
{
	class MessageSystem;
	class NetworkSystem;
}
namespace VadonEditor::Simulator
{
	class PluginManager;
}
namespace VadonEditor::UI
{
	class MainWindow;
}

class QCoreApplication;

namespace VadonEditor::Core
{
	struct EditorSettings;

	class Application
	{
	public:
		Application(int argc, char* argv[]);
		~Application();

		int exec();

		const EditorSettings& get_settings() const;

		Network::MessageSystem& get_message_system();
		Network::NetworkSystem& get_network_system();

		Simulator::PluginManager& get_plugin_manager();

		UI::MainWindow* get_main_window();
	private:
		bool initialize();
		void about_to_quit();

		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif