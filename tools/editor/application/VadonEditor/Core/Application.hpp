#ifndef VADONEDITOR_CORE_APPLICATION_HPP
#define VADONEDITOR_CORE_APPLICATION_HPP
#include <memory>
namespace VadonEditor::Network
{
	class NetworkSystem;
}
namespace VadonEditor::Simulator
{
	class PluginManager;
}
namespace VadonEditor::UI
{
	class UISystem;
}

class QCoreApplication;

namespace VadonEditor::Core
{
	struct CommandLineParameters;
	class Logger;
	class ProjectManager;

	class Application
	{
	public:
		static constexpr const char* c_org_name = "VadonEngine";
		static constexpr const char* c_app_name = "Vadon Editor";

		Application(int argc, char* argv[]);
		~Application();

		int exec();

		const CommandLineParameters& get_command_line_parameters() const;

		Logger& get_logger();
		ProjectManager& get_project_manager();

		Network::NetworkSystem& get_network_system();

		Simulator::PluginManager& get_plugin_manager();

		UI::UISystem& get_ui_system();

		QCoreApplication* get_qt_application() const;
	private:
		bool initialize();

		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif