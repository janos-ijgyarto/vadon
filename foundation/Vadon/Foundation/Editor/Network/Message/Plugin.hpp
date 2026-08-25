#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_PLUGIN_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_PLUGIN_HPP
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
namespace Vadon
{
	namespace Foundation
	{
		// FIXME: some other way to ensure the messages are targeted?
		enum class EditorPluginMessageSource
		{
			ASSET_SERVER,
			SIMULATOR,
		};

		enum class EditorPluginMessageType : uint32
		{
			PLUGIN_INIT,
			PLUGIN_SHUTDOWN,
			PLUGIN_LOG
		};

		struct EditorPluginMessageHeader
		{
			EditorPluginMessageSource plugin_type;
			EditorPluginMessageType message_type;
		};

		struct EditorPluginMessageInit : public EditorPluginMessageHeader
		{
			uint32 error_code;
		};

		struct EditorPluginMessageShutdown : public EditorPluginMessageHeader
		{
			// TODO: any other message data?
			uint32 exit_code;
		};

		// FIXME: could generalize this, create an interface for RPC so other apps can
		// "listen in" on the engine logs
		struct EditorPluginMessageLog : public EditorPluginMessageHeader
		{
			enum Type
			{
				LOG_INFO,
				LOG_WARNING,
				LOG_ERROR
			};

			Type log_type;
			uint32 length;
		};
	}
}
#endif