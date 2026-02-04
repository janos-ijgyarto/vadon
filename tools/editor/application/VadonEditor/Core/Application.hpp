#ifndef VADONEDITOR_CORE_APPLICATION_HPP
#define VADONEDITOR_CORE_APPLICATION_HPP
#include <memory>
namespace VadonEditor::Network
{
	class NetworkSystem;
}
namespace VadonEditor::Simulator
{
	class Simulator;
}
namespace VadonEditor::UI
{
	class UISystem;
}

class QCoreApplication;

namespace VadonEditor::Core
{
	struct Configuration;

	class Logger;
	class PluginManager;
	class ProjectManager;

	class Application
	{
	public:
		static constexpr const char* c_org_name = "VadonEngine";
		static constexpr const char* c_app_name = "Vadon Editor";

		Application(int argc, char* argv[]);
		~Application();

		int exec();

		const Configuration& get_configuration() const;

		Logger& get_logger();
		PluginManager& get_plugin_manager();
		ProjectManager& get_project_manager();

		Network::NetworkSystem& get_network_system();

		Simulator::Simulator& get_simulator();

		UI::UISystem& get_ui_system();

		QCoreApplication* get_qt_application() const;
		void request_quit(int exit_code);
	private:
		bool initialize();

		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif