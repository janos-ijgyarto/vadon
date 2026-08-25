#ifndef VADONEDITOR_CORE_APPLICATION_HPP
#define VADONEDITOR_CORE_APPLICATION_HPP
#include <memory>
namespace VadonEditor::Model
{
	class ModelSystem;
}
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
class QSettings;

namespace VadonEditor::Core
{
	struct Configuration;

	class AssetManager;
	class AssetServer;
	class Logger;
	class PluginManager;
	class ProjectManager;

	class Application
	{
	public:
		static constexpr const char* c_org_name = "VadonEngine";
		static constexpr const char* c_app_name = "Vadon Editor";

		static QSettings get_app_settings();

		Application(int argc, char* argv[]);
		~Application();

		int exec();

		const Configuration& get_configuration() const;

		AssetManager& get_asset_manager();
		AssetServer& get_asset_server();
		Logger& get_logger();
		PluginManager& get_plugin_manager();
		ProjectManager& get_project_manager();

		Model::ModelSystem& get_model_system();

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