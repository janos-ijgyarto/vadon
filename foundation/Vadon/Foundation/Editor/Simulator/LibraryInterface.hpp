#ifndef VADON_FOUNDATION_EDITOR_SIMULATOR_LIBRARYINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_SIMULATOR_LIBRARYINTERFACE_HPP

#ifndef VADON_EDITOR_SIMULATOR_PLUGIN_IMPLEMENTATION
#define VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT
#else
#define VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT __declspec(dllexport)
#endif

namespace Vadon
{
    namespace Foundation
    {
        class EditorSimulatorPluginInterface;
        class EditorSimulatorInterface;
        class TypeMetadataRegistry;
    }
}

using VadonEditorSimulatorPluginInterface = Vadon::Foundation::EditorSimulatorPluginInterface;
using VadonEditorSimulatorInterface = Vadon::Foundation::EditorSimulatorInterface;
using VadonTypeMetadataRegistry = Vadon::Foundation::TypeMetadataRegistry;

extern "C" {
    VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT VadonEditorSimulatorPluginInterface* VadonEditorSimulatorPluginEntrypoint(VadonEditorSimulatorInterface* simulator);
    VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT void VadonEditorSimulatorPluginExit(VadonEditorSimulatorPluginInterface* interface);

    // FIXME: 
    VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT void VadonEditorPluginExportDataSchema(VadonTypeMetadataRegistry* registry);
}

#endif