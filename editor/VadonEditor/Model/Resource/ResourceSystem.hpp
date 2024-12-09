#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <unordered_map>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource;

	using ResourcePath = Vadon::Scene::ResourcePath;

	struct ResourceInfo
	{
		Vadon::Scene::ResourceInfo info;
		Vadon::Scene::ResourceHandle handle;
	};

	class ResourceSystem
	{
	public:
		Resource* create_resource(Vadon::Utilities::TypeID resource_type);
		Resource* get_resource(Vadon::Scene::ResourceHandle resource_handle);

		Resource* import_resource(const ResourcePath& path);

		std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const;

		void remove_resource(Resource* resource);
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();
		bool load_project_resources();

		Resource* find_resource(Vadon::Scene::ResourceHandle resource_handle) { return const_cast<Resource*>(std::as_const(*this).find_resource(resource_handle)); }
		const Resource* find_resource(Vadon::Scene::ResourceHandle resource_handle) const;
		Resource* internal_create_resource(Vadon::Scene::ResourceHandle resource_handle);

		Core::Editor& m_editor;
		std::unordered_map<uint64_t, Resource> m_resource_lookup;

		friend class ModelSystem;
	};
}
#endif