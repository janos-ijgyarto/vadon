#ifndef VADONEDITOR_CORE_ASSET_ASSETSERVER_HPP
#define VADONEDITOR_CORE_ASSET_ASSETSERVER_HPP
#include <Vadon/Foundation/Editor/Asset/AssetServerInterface.hpp>
#include <memory>
#include <QString>
namespace Vadon::Foundation
{
	class EditorAssetServerPluginInterface;
}
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Core
{
	struct AssetServerSettings
	{
		bool debug_break_on_init = false;
		QString configuration_name;
	};

	// TODO: "hide" the asset server interface so it's only available internally?
	class AssetServer : public Vadon::Foundation::EditorAssetServerInterface
	{
	public:
		~AssetServer();

		::Vadon::Foundation::EditorAssetServerPluginInterface* get_plugin_interface() const;

		void dispatch_message_to_editor(const char* data, size_t size) override;

		bool run_asset_server(const AssetServerSettings& settings);
		bool is_running() const;
		void stop_asset_server();

		void export_project_data(const QString& output_path);
	private:
		AssetServer(Core::Application& application);

		bool initialize();
		void shutdown();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif