#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <Vadon/Scene/Resource/Database.hpp>
#include <unordered_map>
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
		ResourcePath path;
	};

	class EditorResourceDatabase
	{
	public:
		virtual ResourceID find_resource_id(const ResourcePath& path) const = 0;
		virtual ResourceInfo find_resource_info(ResourceID resource_id) const = 0;

		virtual void set_resource_path(ResourceID resource_id, const ResourcePath& path) = 0;

		virtual ResourceID import_resource(const ResourcePath& path) = 0;

		virtual	std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const = 0;
	};

	class ResourceSystem
	{
	public:
		Resource* create_resource(Vadon::Utilities::TypeID resource_type);
		Resource* get_resource(ResourceID resource_id);
		Resource* get_resource(Vadon::Scene::ResourceHandle resource_handle);
		void remove_resource(Resource* resource);

		EditorResourceDatabase& get_database() { return m_database; }
	private:
		class EditorResourceDatabaseImpl : public Vadon::Scene::ResourceDatabase, public EditorResourceDatabase
		{
		public:
			EditorResourceDatabaseImpl(Core::Editor& editor);

			bool save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle) override;
			Vadon::Scene::ResourceHandle load_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceID resource_id) override;

			ResourceID find_resource_id(const ResourcePath& path) const override;
			ResourceInfo find_resource_info(ResourceID resource_id) const override;

			void set_resource_path(ResourceID resource_id, const ResourcePath& path) override;

			ResourceID import_resource(const ResourcePath& path) override;

			std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const override;
		private:
			const ResourceInfo* internal_find_resource_info(ResourceID resource_id) const;

			void internal_import_resource(const Vadon::Scene::ResourceInfo& resource_info, const ResourcePath& path);
			void internal_remove_resource(ResourceID resource_id);

			Core::Editor& m_editor;

			std::unordered_map<ResourceID, ResourceInfo> m_resource_file_lookup;
			std::unordered_map<std::string, ResourceID> m_resource_id_lookup;
		};

		ResourceSystem(Core::Editor& editor);

		bool initialize();
		bool load_project_resources();

		Core::Editor& m_editor;
		std::unordered_map<ResourceID, Resource> m_resource_lookup;
		EditorResourceID m_resource_id_counter;

		EditorResourceDatabaseImpl m_database;

		friend class ModelSystem;
	};
}
#endif