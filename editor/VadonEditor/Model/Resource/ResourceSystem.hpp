#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <Vadon/Core/File/Database.hpp>
#include <Vadon/Scene/Resource/Database.hpp>
#include <functional>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource;

	struct ResourceInfo
	{
		Vadon::Scene::ResourceInfo info;
		std::string path;
	};

	class EditorResourceDatabase
	{
	public:
		virtual const ResourceInfo* find_resource_info(ResourceID resource_id) const = 0;

		// NOTE: this is used for newly created resources that have not yet been saved to a file
		virtual bool save_resource_as(ResourceID resource_id, std::string_view path) = 0;

		virtual	std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const = 0;
	};

	class ResourceSystem
	{
	public:
		// FIXME: extend this so we can get info on what was modified!
		using EditCallback = std::function<void(ResourceID)>;

		Resource* create_resource(Vadon::Utilities::TypeID resource_type);
		VADONEDITOR_API Resource* get_resource(ResourceID resource_id);
		Resource* get_resource(Vadon::Scene::ResourceHandle resource_handle);
		void remove_resource(Resource* resource);

		EditorResourceDatabase& get_database() { return m_database; }

		VADONEDITOR_API void register_edit_callback(EditCallback callback);
		void resource_edited(const Resource& resource);
	private:
		class EditorResourceDatabaseImpl : public Vadon::Scene::ResourceDatabase, public EditorResourceDatabase
		{
		public:
			enum class FileDatabaseType
			{
				RESOURCE,
				ASSET_FILE,
				TYPE_COUNT
			};

			EditorResourceDatabaseImpl(Core::Editor& editor);

			bool initialize();

			bool save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle) override;
			Vadon::Scene::ResourceHandle load_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceID resource_id) override;

			Vadon::Core::FileInfo get_file_resource_info(Vadon::Scene::ResourceID resource_id) const override;
			bool load_file_resource_data(Vadon::Scene::ResourceSystem& resource_system, ResourceID resource_id, Vadon::Core::RawFileDataBuffer& file_data) override;

			const ResourceInfo* find_resource_info(ResourceID resource_id) const override;

			bool save_resource_as(ResourceID resource_id, std::string_view path) override;

			ResourceID import_resource(std::string_view path);

			std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const override;

			Vadon::Core::FileDatabaseHandle get_database(FileDatabaseType type) const { return m_file_databases[static_cast<size_t>(type)]; }
		private:
			void internal_import_resource(const Vadon::Scene::ResourceInfo& resource_info, std::string_view path);
			bool import_asset_file(ResourceID file_id);

			Core::Editor& m_editor;
			std::unordered_map<Vadon::Scene::ResourceID, ResourceInfo> m_resource_info_lookup;

			std::array<Vadon::Core::FileDatabaseHandle, static_cast<size_t>(FileDatabaseType::TYPE_COUNT)> m_file_databases;
		};

		ResourceSystem(Core::Editor& editor);

		bool initialize();
		bool load_project_resources();
		bool export_project_resources(std::string_view output_path);

		Core::Editor& m_editor;
		std::unordered_map<ResourceID, Resource> m_resource_lookup;
		EditorResourceID m_resource_id_counter;

		EditorResourceDatabaseImpl m_database;

		std::vector<EditCallback> m_edit_callbacks;

		friend class ModelSystem;
	};
}
#endif