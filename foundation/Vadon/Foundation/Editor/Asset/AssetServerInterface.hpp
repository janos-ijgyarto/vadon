#ifndef VADON_FOUNDATION_EDITOR_ASSET_ASSETSERVERINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_ASSET_ASSETSERVERINTERFACE_HPP
namespace Vadon
{
	namespace Foundation
	{
		struct AssetServerToolchainConfiguration
		{
			const char* export_path;
		};

		class EditorAssetServerInterface
		{
		public:
			virtual ~EditorAssetServerInterface() {}
			virtual void dispatch_message_to_editor(const char* data, size_t size) = 0;

			virtual AssetServerToolchainConfiguration get_toolchain_configuration() const = 0;
		};
	}
}
#endif