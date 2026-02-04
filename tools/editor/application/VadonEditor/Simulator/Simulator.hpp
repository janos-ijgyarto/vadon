#ifndef VADONEDITOR_SIMULATOR_SIMULATOR_HPP
#define VADONEDITOR_SIMULATOR_SIMULATOR_HPP
#include <Vadon/Foundation/Editor/Simulator/SimulatorInterface.hpp>
#include <memory>
namespace Vadon::Foundation
{
	class EditorSimulatorPluginInterface;
}
namespace VadonEditor::Core
{
	class Application;
}
class QString;
namespace VadonEditor::Simulator
{
	struct SimulatorSettings
	{
		bool debug_break_on_init = false;
	};

	// TODO: "hide" the simulator interface so it's only available internally?
	class Simulator : public Vadon::Foundation::EditorSimulatorInterface
	{
	public:
		~Simulator();

		::Vadon::Foundation::EditorSimulatorPluginInterface* get_plugin_interface() const;

		void dispatch_message_to_editor(const char* data, size_t size) override;

		bool run_simulator(const SimulatorSettings& settings);
		void stop_simulator();
	private:
		Simulator(Core::Application& application);

		bool initialize();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif