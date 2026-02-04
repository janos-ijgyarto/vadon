#ifndef VADON_FOUNDATION_EDITOR_SIMULATOR_PLUGININTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_SIMULATOR_PLUGININTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		class EditorSimulatorInterface;
		class TypeMetadataRegistry;

		class EditorSimulatorPluginInterface
		{
		public:
			virtual ~EditorSimulatorPluginInterface() {}

			EditorSimulatorInterface& get_simulator() { return m_simulator; }

			virtual bool initialize() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void process_message_from_editor(const char* data, size_t size) = 0;

			virtual void editor_connected() = 0;
			virtual void editor_disconnected() = 0;

			virtual const TypeMetadataRegistry& get_metadata_registry() const = 0;
		protected:
			EditorSimulatorPluginInterface(EditorSimulatorInterface& simulator) : m_simulator(simulator) {}

			EditorSimulatorInterface& m_simulator;
		};
	}
}
#endif