#ifndef VADON_FOUNDATION_EDITOR_PLUGININTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_PLUGININTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		class EditorSimulatorInterface;
		struct EditorMessageHeader;
		class TypeMetadataRegistry;

		class EditorPluginInterface
		{
		public:
			virtual ~EditorPluginInterface() {}

			virtual void update() = 0;
			virtual void process_message_from_editor(const EditorMessageHeader& header, const void* data) = 0;

			virtual void editor_connected() = 0;
			virtual void editor_disconnected() = 0;

			virtual const TypeMetadataRegistry& get_metadata_registry() const = 0;
		protected:
			EditorPluginInterface(EditorSimulatorInterface& simulator) : m_simulator(simulator) {}

			EditorSimulatorInterface& m_simulator;
		};
	}
}
#endif