#ifndef VADONEDITOR_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_SCENE_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Scene/Resource/Resource.hpp>
#include <functional>
namespace VadonEditor::Scene
{
	class ResourceSystem
	{
	public:
		// FIXME: extend this so we can get info on what was modified!
		using EditCallback = std::function<void(Vadon::Scene::ResourceID)>;

		VADONEDITOR_API Resource* get_resource(const Vadon::Scene::ResourceID& resource_id);

		VADONEDITOR_API void register_edit_callback(EditCallback callback);
	private:
		std::unordered_map<Vadon::Scene::ResourceID, Resource> m_resource_lookup;

		std::vector<EditCallback> m_edit_callbacks;
	};
}
#endif