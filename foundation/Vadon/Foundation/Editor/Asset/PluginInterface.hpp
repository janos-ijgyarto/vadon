#ifndef VADON_FOUNDATION_EDITOR_ASSET_PLUGININTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_ASSET_PLUGININTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		class EditorAssetServerInterface;

		class EditorAssetServerPluginInterface
		{
		public:
			virtual ~EditorAssetServerPluginInterface() {}

			EditorAssetServerInterface& get_editor_interface() { return m_editor_interface; }

			virtual bool initialize(const char* project_path) = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void process_message_from_editor(const char* data, size_t size) = 0;

			virtual void editor_connected() = 0;
			virtual void editor_disconnected() = 0;
		protected:
			EditorAssetServerPluginInterface(EditorAssetServerInterface& editor_interface) : m_editor_interface(editor_interface) {}

			EditorAssetServerInterface& m_editor_interface;
		};
	}
}
#endif