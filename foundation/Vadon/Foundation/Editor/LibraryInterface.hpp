#ifndef VADON_FOUNDATION_EDITOR_LIBRARYINTERFACE_HPP
#define VADON_FOUNDATION_EDITOR_LIBRARYINTERFACE_HPP

#ifndef VADON_EDITOR_SIMULATOR_PLUGIN_IMPLEMENTATION
#define VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT
#else
#define VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT __declspec(dllexport)
#endif

namespace Vadon
{
    namespace Foundation
    {
        class EditorPluginInterface;
        class EditorSimulatorInterface;
    }
}

using VadonEditorPluginInterface = Vadon::Foundation::EditorPluginInterface;
using VadonEditorSimulatorInterface = Vadon::Foundation::EditorSimulatorInterface;

extern "C" {
    VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT VadonEditorPluginInterface* VadonEditorPluginEntrypoint(VadonEditorSimulatorInterface* simulator);
    VADON_EDITOR_SIMULATOR_PLUGIN_DLLEXPORT void VadonEditorPluginExit(VadonEditorPluginInterface* interface);
}

#endif