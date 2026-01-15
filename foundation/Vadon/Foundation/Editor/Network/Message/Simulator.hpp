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

		struct EditorSimulatorMessageShutdown : public EditorSimulatorMessageHeader
		{
			// TODO: exit code, or some other metadata to explain shutdown reason?
		};
	}
}
#endif