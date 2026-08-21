#include <VadonDemo/Core/AssetServer.hpp>
#include <VadonDemo/Core/Editor.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <Vadon/Foundation/Editor/Asset/LibraryInterface.hpp>
#include <Vadon/Foundation/Editor/Simulator/LibraryInterface.hpp>

namespace
{
	Vadon::Core::EngineEnvironment* s_engine_environment = nullptr;
}

VadonEditorSimulatorPluginInterface* VadonEditorPluginEntrypoint(VadonEditorSimulatorInterface* simulator)
{
	if (s_engine_environment != nullptr)
	{
		// TODO: report error!
		return nullptr;
	}

	s_engine_environment = new Vadon::Core::EngineEnvironment();
	VadonDemo::Core::Editor::init_environment(*s_engine_environment);

	VadonDemo::Core::Editor* editor = new VadonDemo::Core::Editor(*simulator);
	return editor;
}

void VadonEditorPluginExit(VadonEditorSimulatorPluginInterface* interface)
{
	delete interface;

	delete s_engine_environment;
	s_engine_environment = nullptr;
}

void VadonEditorPluginExportDataSchema(VadonTypeMetadataRegistry* registry)
{
	Vadon::Core::EngineEnvironment engine_environment;
	VadonDemo::Core::Editor::init_environment(engine_environment);

	VadonDemo::Core::Editor::register_type_metadata(*registry);

	// TODO: shutdown environment?
}

VadonEditorAssetServerPluginInterface* VadonEditorAssetServerPluginEntrypoint(VadonEditorAssetServerInterface* asset_server)
{
	if (s_engine_environment != nullptr)
	{
		// TODO: report error!
		return nullptr;
	}

	s_engine_environment = new Vadon::Core::EngineEnvironment();
	VadonDemo::Core::AssetServer::init_environment(*s_engine_environment);

	VadonDemo::Core::AssetServer* demo_asset_server = new VadonDemo::Core::AssetServer(*asset_server);
	return demo_asset_server;
}

void VadonEditorAssetServerPluginExit(VadonEditorAssetServerPluginInterface* interface)
{
	delete interface;

	delete s_engine_environment;
	s_engine_environment = nullptr;
}