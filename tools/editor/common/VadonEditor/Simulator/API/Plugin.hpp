#ifndef VADONEDITOR_SIMULATOR_API_PLUGIN_HPP
#define VADONEDITOR_SIMULATOR_API_PLUGIN_HPP
namespace VadonEditor::Network
{
	struct MessageHeader;
}
namespace VadonEditor::Simulator
{
	class SimulatorInterface;

	// This object must be implemented by plugins so the simulator can communicate with them
	class PluginInterface
	{
	public:
		virtual ~PluginInterface() {}

		virtual void update() = 0;
		virtual void process_message_from_editor(const VadonEditor::Network::MessageHeader& header, const void* data) = 0;

		virtual void editor_connected() = 0;
		virtual void editor_disconnected() = 0;
	protected:
		PluginInterface(SimulatorInterface& simulator) : m_simulator(simulator) {}

		SimulatorInterface& m_simulator;
	};
}
#endif
