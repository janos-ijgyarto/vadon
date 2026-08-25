#ifndef VADON_FOUNDATION_EDITOR_ASSET_LIBRARYINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_ASSET_LIBRARYINTERFACE_HPP

#ifndef VADON_EDITOR_ASSET_SERVER_PLUGIN_IMPLEMENTATION
#define VADON_EDITOR_ASSET_SERVER_PLUGIN_DLLEXPORT
#else
#define VADON_EDITOR_ASSET_SERVER_PLUGIN_DLLEXPORT __declspec(dllexport)
#endif

namespace Vadon
{
    namespace Foundation
    {
        class EditorAssetServerPluginInterface;
        class EditorAssetServerInterface;
    }
}

using VadonEditorAssetServerPluginInterface = Vadon::Foundation::EditorAssetServerPluginInterface;
using VadonEditorAssetServerInterface = Vadon::Foundation::EditorAssetServerInterface;

extern "C" {
    VADON_EDITOR_ASSET_SERVER_PLUGIN_DLLEXPORT VadonEditorAssetServerPluginInterface* VadonEditorAssetServerPluginEntrypoint(VadonEditorAssetServerInterface* asset_server);
    VADON_EDITOR_ASSET_SERVER_PLUGIN_DLLEXPORT void VadonEditorAssetServerPluginExit(VadonEditorAssetServerPluginInterface* interface);
}

#endif