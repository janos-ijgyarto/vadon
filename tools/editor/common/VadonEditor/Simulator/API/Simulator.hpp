#ifndef VADONEDITOR_SIMULATOR_API_SIMULATOR_HPP
#define VADONEDITOR_SIMULATOR_API_SIMULATOR_HPP
namespace VadonEditor::Simulator
{
	// This object can be used by client code to access systems in the simulator (e.g communicating with the editor)
	class SimulatorInterface
	{
	public:
		virtual ~SimulatorInterface() {}
		virtual void dispatch_message_to_editor(const void* data, size_t size) = 0;
	};
}
#endif
