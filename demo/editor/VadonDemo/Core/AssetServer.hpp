#ifndef VADONDEMO_CORE_ASSETSERVER_HPP
#define VADONDEMO_CORE_ASSETSERVER_HPP
#include <VadonEditor/Core/AssetServer.hpp>
#include <VadonEditor/Core/Logger.hpp>

#include <Vadon/Core/Core.hpp>

#include <Vadon/Foundation/Editor/Asset/PluginInterface.hpp>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace VadonDemo::Core
{
	class AssetServerLogger : public VadonEditor::Core::Logger
	{
	public:
		AssetServerLogger(::Vadon::Foundation::EditorAssetServerInterface& asset_server);
	protected:
		void dispatch_message_data(const char* data, size_t size) override;
	private:
		::Vadon::Foundation::EditorAssetServerInterface& m_asset_server;
	};

	class AssetServer : public ::Vadon::Foundation::EditorAssetServerPluginInterface
	{
	public:
		AssetServer(::Vadon::Foundation::EditorAssetServerInterface& asset_server);
		~AssetServer();

		static void init_environment(Vadon::Core::EngineEnvironment& environment);

		bool initialize(const char* project_path) override;
		void shutdown() override;

		VadonEditor::Core::AssetServer& get_common_asset_server() { return m_common_asset_server; }

		Vadon::Core::EngineCoreInterface& get_engine_core() { return *m_engine_core; }

		void process_message_from_editor(const char* data, size_t size) override;

		void editor_connected() override;
		void editor_disconnected() override;
	private:
		bool project_loaded();

		Vadon::Core::EngineCorePtr m_engine_core;
		VadonEditor::Core::AssetServer m_common_asset_server;
		AssetServerLogger m_logger;
	};
}
#endif