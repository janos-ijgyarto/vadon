#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_SIMULATOR_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_SIMULATOR_HPP
#include <Vadon/Foundation/Editor/Simulator/SimulatorInterface.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorSimulatorMessageType : uint32
		{
			REGISTER_SETTING,
			UPDATE_SETTING
		};

		struct EditorSimulatorMessageHeader
		{
			EditorSimulatorMessageType message_type;
		};

		struct EditorSimulatorMessageRegisterSetting : public EditorSimulatorMessageHeader
		{
			SimulatorSetting setting;
			uint32 label_length;
		};

		struct EditorSimulatorMessageUpdateSetting : public EditorSimulatorMessageHeader
		{
			UUID setting_id;
			uint32 data_size;
		};
	}
}
#endif