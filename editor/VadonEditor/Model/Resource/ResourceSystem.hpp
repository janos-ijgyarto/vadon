#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/Model/Resource/Resource.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
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
		std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const;
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();
		bool load_project_resources();

		Resource* find_resource(Vadon::Scene::ResourceHandle resource_handle);
		Resource* internal_create_resource(Vadon::Scene::ResourceHandle resource_handle);

		Core::Editor& m_editor;
		std::unordered_map<uint64_t, Resource> m_resource_lookup;

		friend class ModelSystem;
	};
}
#endif