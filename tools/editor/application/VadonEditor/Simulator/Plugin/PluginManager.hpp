#ifndef VADONEDITOR_SIMULATOR_PLUGIN_PLUGINMANAGER_HPP
#define VADONEDITOR_SIMULATOR_PLUGIN_PLUGINMANAGER_HPP
#include <VadonEditor/Simulator/API/Simulator.hpp>
#include <memory>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Simulator
{
	class PluginInterface;

	class PluginManager : public VadonEditor::Simulator::SimulatorInterface
	{
	public:
		PluginManager(Core::Application& application);
		~PluginManager();

		bool initialize();
		void update();
		void shutdown();

		PluginInterface* get_plugin() const;

		void dispatch_message_to_editor(const void* data, size_t size) override;
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif