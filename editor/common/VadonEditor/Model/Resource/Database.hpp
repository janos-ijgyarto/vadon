#ifndef VADONEDITOR_MODEL_RESOURCE_DATABASE_HPP
#define VADONEDITOR_MODEL_RESOURCE_DATABASE_HPP
#include <Vadon/Core/File/Database.hpp>
#include <Vadon/Model/Resource/Database.hpp>

#include <array>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace VadonEditor::Core
{
	class ProjectManager;
}
namespace VadonEditor::Model
{
	struct ResourceDatabaseEntry
	{
		Vadon::Model::ResourceInfo base_info;
		std::string path;
	};

	class ResourceDatabase : public Vadon::Model::ResourceDatabase
	{
	public:
		enum class FileDatabaseType
		{
			RESOURCE,
			ASSET_FILE,
			TYPE_COUNT
		};

		ResourceDatabase(Vadon::Core::EngineCoreInterface& engine_core, VadonEditor::Core::ProjectManager& project_manager);

		bool initialize();

		bool save_resource(Vadon::Model::ResourceSystem& resource_system, Vadon::Model::ResourceHandle resource_handle) override;
		Vadon::Model::ResourceHandle load_resource(Vadon::Model::ResourceSystem& resource_system, Vadon::Model::ResourceID resource_id) override;

		Vadon::Core::FileInfo get_file_resource_info(Vadon::Model::ResourceID resource_id) const override;
		bool load_file_resource_data(Vadon::Model::ResourceSystem& /*resource_system*/, Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) override { return load_file_resource_data(resource_id, file_data); }

		bool load_resource_data(Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) const;
		bool load_file_resource_data(Vadon::Model::ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) const;

		const ResourceDatabaseEntry* find_resource_entry(Vadon::Model::ResourceID resource_id) const;
		Vadon::Model::ResourceID import_resource(std::string_view path);
		
		bool import_project_resources();
		std::vector<Vadon::Model::ResourceID> get_resource_list() const;

		Vadon::Core::FileDatabaseHandle get_database(FileDatabaseType type) const { return m_file_databases[static_cast<size_t>(type)]; }

		static bool sanitize_editor_resource_file(Vadon::Core::RawFileDataBuffer& file_data);
	private:
		void internal_import_resource(const Vadon::Model::ResourceInfo& resource_info, std::string_view path);
		bool import_asset_file(Vadon::Model::ResourceID file_id);

		Vadon::Core::EngineCoreInterface& m_engine_core;
		VadonEditor::Core::ProjectManager& m_project_manager;

		std::unordered_map<Vadon::Model::ResourceID, ResourceDatabaseEntry> m_resource_entry_lookup;

		std::array<Vadon::Core::FileDatabaseHandle, static_cast<size_t>(FileDatabaseType::TYPE_COUNT)> m_file_databases;
	};
}
#endif