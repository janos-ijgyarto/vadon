#ifndef VADONEDITOR_SIMULATOR_API_LIBRARYINTERFACE_HPP
#define VADONEDITOR_SIMULATOR_API_LIBRARYINTERFACE_HPP

#ifndef VADONEDITOR_SIMULATOR_PLUGIN_IMPLEMENTATION
#define VADONEDITOR_SIMULATOR_PLUGIN_DLLEXPORT
#else
#define VADONEDITOR_SIMULATOR_PLUGIN_DLLEXPORT __declspec(dllexport)
#endif

namespace VadonEditor::Simulator
{
    class PluginInterface;
    class SimulatorInterface;
}

using VadonEditorPluginInterface = VadonEditor::Simulator::PluginInterface;
using VadonEditorSimulatorInterface = VadonEditor::Simulator::SimulatorInterface;

extern "C" {
    VADONEDITOR_SIMULATOR_PLUGIN_DLLEXPORT VadonEditorPluginInterface* VadonEditorPluginEntrypoint(VadonEditorSimulatorInterface* simulator);
    VADONEDITOR_SIMULATOR_PLUGIN_DLLEXPORT void VadonEditorPluginExit(VadonEditorPluginInterface* interface);
}

#endif