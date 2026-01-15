#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_PLATFORM_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_PLATFORM_HPP
#include <Vadon/Foundation/Utilities/Numeric.hpp>
#include <Vadon/Foundation/Platform/Event/Event.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorPlatformMessageType
		{
			MANAGER_WINDOW_REQUEST,
			PLATFORM_EVENT
		};

		struct EditorPlatformMessageHeader
		{
			EditorPlatformMessageType message_type;
		};

		struct EditorPlatformManagerWindowRequest : public EditorPlatformMessageHeader
		{
			// NOTE: ID is provided by client, editor returns handle
			int id;
			uintptr_t handle;
		};

		struct EditorPlatformEventHeader : public EditorPlatformMessageHeader
		{
			PlatformEventType event_type;
		};
	}
}
#endif