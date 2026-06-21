#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_SIMULATOR_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_SIMULATOR_HPP
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorSimulatorMessageType : uint32
		{
			SIMULATOR_INIT,
			SIMULATOR_LOG,
			SIMULATOR_SHUTDOWN
		};

		struct EditorSimulatorMessageHeader
		{
			EditorSimulatorMessageType message_type;
		};

		struct EditorSimulatorMessageInit : public EditorSimulatorMessageHeader
		{
			uint32 error_code;
		};

		// FIXME: could generalize this, create an interface for RPC so other apps can
		// "listen in" on the engine logs
		struct EditorSimulatorMessageLog : public EditorSimulatorMessageHeader
		{
			enum Type
			{
				INFO,
				WARNING,
				ERROR
			};

			Type log_type;
			uint32 length;
		};

		struct EditorSimulatorMessageShutdown : public EditorSimulatorMessageHeader
		{
			// TODO: any other message data?
			uint32 exit_code;
		};
	}
}
#endif