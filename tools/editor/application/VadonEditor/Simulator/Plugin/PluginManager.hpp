#ifndef VADONEDITOR_SIMULATOR_PLUGIN_PLUGINMANAGER_HPP
#define VADONEDITOR_SIMULATOR_PLUGIN_PLUGINMANAGER_HPP
#include <Vadon/Foundation/Editor/SimulatorInterface.hpp>
#include <memory>
namespace Vadon::Foundation
{
	class EditorPluginInterface;
}
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Simulator
{
	class PluginManager : public Vadon::Foundation::EditorSimulatorInterface
	{
	public:
		PluginManager(Core::Application& application);
		~PluginManager();

		bool initialize();
		void shutdown();

		::Vadon::Foundation::EditorPluginInterface* get_plugin() const;

		void dispatch_message_to_editor(const char* data, size_t size) override;
	private:
		bool run_simulator();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif