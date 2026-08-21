#include <VadonDemo/Core/AssetServer.hpp>

#include <VadonDemo/Core/Core.hpp>

#include <VadonEditor/Core/AssetServer.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/CoreConfiguration.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/Foundation/Editor/Asset/AssetServerInterface.hpp>
#include <Vadon/Foundation/TypeInfo/MetadataRegistry.hpp>

namespace VadonDemo::Core
{
	AssetServerLogger::AssetServerLogger(::Vadon::Foundation::EditorAssetServerInterface& asset_server)
		: Logger(::Vadon::Foundation::EditorPluginMessageSource::ASSET_SERVER)
		, m_asset_server(asset_server)
	{
	}

	void AssetServerLogger::dispatch_message_data(const char* data, size_t size)
	{
		m_asset_server.dispatch_message_to_editor(data, size);
	}

	AssetServer::AssetServer(::Vadon::Foundation::EditorAssetServerInterface& asset_server)
		: ::Vadon::Foundation::EditorAssetServerPluginInterface(asset_server)
		, m_engine_core(Vadon::Core::create_engine_core())
		, m_common_asset_server(*m_engine_core)
		, m_logger(asset_server)
	{
		Vadon::Core::EngineEnvironment::set_logger(&m_logger);
	}

	AssetServer::~AssetServer()
	{
		Vadon::Core::EngineEnvironment::set_logger(nullptr);
	}

	void AssetServer::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		VadonEditor::Core::AssetServer::init_environment(environment);
		VadonDemo::Core::Core::init_environment(environment);
	}

	bool AssetServer::initialize(const char* project_path)
	{
        // TODO: use command line to set up configs!
        Vadon::Core::CoreConfiguration engine_config;
		engine_config.common_config.render_config.flags |= Vadon::Core::RenderConfigurationFlags::DISABLE_RENDERING;
        if (m_engine_core->initialize(engine_config) == false)
        {
            // TODO: error!
            return false;
        }

        // Initialize the asset server
        if (m_common_asset_server.initialize() == false)
        {
            return false;
        }

		// TODO: should we still use metadata?
		Vadon::Core::register_engine_types();

		// Use null registry, since we don't need the metadata while processing assets
		// FIXME: we might still want it in debug builds?
		::Vadon::Foundation::NullMetadataRegistry null_metadata_registry;
		VadonDemo::Core::Core::register_types(null_metadata_registry);

        if (m_common_asset_server.load_project(project_path) == false)
        {
            return false;
        }

        if (project_loaded() == false)
        {
            return false;
        }

        return true;
	}

	void AssetServer::shutdown()
	{

	}

	void AssetServer::process_message_from_editor(const char* data, size_t size)
	{
		// TODO: any other processing here?
		m_common_asset_server.process_message(data, size);
	}

	void AssetServer::editor_connected()
	{
		// TODO: anything?
	}

	void AssetServer::editor_disconnected()
	{
		// TODO: anything?
	}

	bool AssetServer::project_loaded()
	{
		// TODO: anything?
		return true;
	}
}