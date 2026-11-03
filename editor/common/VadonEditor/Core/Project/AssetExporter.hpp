#ifndef VADONEDITOR_CORE_PROJECT_ASSETEXPORTER_HPP
#define VADONEDITOR_CORE_PROJECT_ASSETEXPORTER_HPP
#include <Vadon/Core/File/File.hpp>
#include <Vadon/Core/File/Database.hpp>
#include <unordered_map>
namespace VadonEditor::Core
{
	class AssetServer;

	class AssetExporter
	{
	private:
		struct CacheEntry
		{
			Vadon::Core::FileMetadata file_metadata;
		};

		using ExporterCache = std::unordered_map<::Vadon::Foundation::UUID, CacheEntry>;
		struct Database
		{
			Vadon::Core::FileDatabaseHandle file_db_handle;
			ExporterCache cache;

			void update_cache(const ::Vadon::Foundation::UUID& file_id, const Vadon::Core::FileInfo& file_info);
			void remove_from_cache(const ::Vadon::Foundation::UUID& file_id);

			bool is_file_up_to_date(const ::Vadon::Foundation::UUID& file_id, const Vadon::Core::FileInfo& current_file_info) const;
		};

		AssetExporter(AssetServer& asset_server);

		bool is_ready() const { return m_output_root.empty() == false; }

		bool initialize();
		bool initialize_output(std::string_view output_path);
		void update();

		bool export_file(const ::Vadon::Foundation::UUID& resource_id);
		bool export_resource_file(const ::Vadon::Foundation::UUID& resource_id);
		bool export_asset_file(const ::Vadon::Foundation::UUID& asset_file_id);

		bool export_project_file();

		bool export_all_project_data();

		void clear_stale_files();

		VadonEditor::Core::AssetServer& m_asset_server;

		std::string m_output_root;

		Vadon::Core::FileMetadata m_project_file_metadata;
		Database m_resource_database;
		Database m_asset_database;

		Vadon::Core::RawFileDataBuffer m_source_file_data_buffer;
		Vadon::Core::RawFileDataBuffer m_exported_file_data_buffer;

		friend AssetServer;
	};
}
#endif