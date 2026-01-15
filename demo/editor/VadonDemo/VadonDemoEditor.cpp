#include <VadonDemo/Core/Editor.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Foundation/Editor/LibraryInterface.hpp>

namespace
{
	Vadon::Core::EngineEnvironment* s_engine_environment = nullptr;
}

VadonEditorPluginInterface* VadonEditorPluginEntrypoint(VadonEditorSimulatorInterface* simulator)
{
	if (s_engine_environment != nullptr)
	{
		// TODO: report error!
		return nullptr;
	}

	s_engine_environment = new Vadon::Core::EngineEnvironment();
	VadonDemo::Core::Editor* editor = new VadonDemo::Core::Editor(*s_engine_environment, *simulator);

	return editor;
}

void VadonEditorPluginExit(VadonEditorPluginInterface* interface)
{
	delete interface;

	delete s_engine_environment;
	s_engine_environment = nullptr;
}