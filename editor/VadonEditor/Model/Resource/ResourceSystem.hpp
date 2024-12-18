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
		Vadon::Scene::ResourceHandle handle;
	};

	class EditorResourceDatabase : public Vadon::Scene::ResourceDatabase
	{
	public:
		bool save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle) override;
		Vadon::Scene::ResourceHandle load_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceID resource_id) override;

		bool save_resource(Vadon::Scene::ResourceHandle resource_handle, const ResourcePath& path);
		Vadon::Scene::ResourceHandle load_resource(const ResourcePath& path);

		ResourceID find_resource_id(const ResourcePath& path);
		ResourcePath find_resource_path(ResourceID resource_id);

		ResourceID import_resource(const ResourcePath& path);
	private:
		EditorResourceDatabase(Core::Editor& editor);

		bool internal_save_resource(Vadon::Scene::ResourceSystem& resource_system, Vadon::Scene::ResourceHandle resource_handle, const ResourcePath& path);
		Vadon::Scene::ResourceHandle internal_load_resource(Vadon::Scene::ResourceSystem& resource_system, const ResourcePath& path);

		ResourcePath find_resource_path(ResourceID resource_id) const;

		Core::Editor& m_editor;

		std::unordered_map<ResourceID, ResourcePath> m_resource_file_lookup;
		std::unordered_map<std::string, ResourceID> m_resource_id_lookup;

		friend class ResourceSystem;
	};

	class ResourceSystem
	{
	public:
		Resource* create_resource(Vadon::Utilities::TypeID resource_type);
		Resource* get_resource(Vadon::Scene::ResourceHandle resource_handle);

		std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const;

		void remove_resource(Resource* resource);

		EditorResourceDatabase& get_database() { return m_database; }
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();
		bool load_project_resources();

		Resource* internal_get_resource(ResourceID resource_id);

		Core::Editor& m_editor;
		std::unordered_map<ResourceID, Resource> m_resource_lookup;
		EditorResourceID m_resource_id_counter;

		EditorResourceDatabase m_database;

		friend class ModelSystem;
	};
}
#endif