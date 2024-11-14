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
	struct ResourceInfo
	{
		Vadon::Scene::ResourceHandle handle;
		std::string path;
	};

	class ResourceSystem
	{
	public:
		Resource* get_resource(Vadon::Scene::ResourceHandle resource_handle);

		std::vector<ResourceInfo> get_resource_list(Vadon::Utilities::TypeID resource_type) const;
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();

		Core::Editor& m_editor;

		std::unordered_map<uint64_t, Resource> m_resource_lookup;

		friend class ModelSystem;
	};
}
#endif