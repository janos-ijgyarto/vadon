#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_ASSETSERVER_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_ASSETSERVER_HPP
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorAssetServerMessageType : uint32
		{
			EXPORT_DATA
		};

		struct EditorAssetServerMessageHeader
		{
			EditorAssetServerMessageType message_type;
		};

		struct EditorAssetServerMessageExportData : public EditorAssetServerMessageHeader
		{
			uint32 output_path_length;
		};
	}
}
#endif