#ifndef VADON_FOUNDATION_EDITOR_ASSET_ASSETSERVERINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_ASSET_ASSETSERVERINTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		class EditorAssetServerInterface
		{
		public:
			virtual ~EditorAssetServerInterface() {}
			virtual void dispatch_message_to_editor(const char* data, size_t size) = 0;
		};
	}
}
#endif