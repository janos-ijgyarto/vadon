#ifndef VADONEDITOR_SCENE_SCENESYSTEM_HPP
#define VADONEDITOR_SCENE_SCENESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Scene/Event.hpp>
#include <VadonEditor/Scene/Scene.hpp>
#include <functional>
namespace VadonEditor::Scene
{
	class SceneSystem
	{
	public:
		using EntityEventCallback = std::function<void(const EntityEvent&)>;
		using ComponentEventCallback = std::function<void(const ComponentEvent&)>;

		VADONEDITOR_API Scene* get_scene(Vadon::Scene::SceneID scene_id);
		// TODO: option to remove scenes?

		VADONEDITOR_API void add_entity_event_callback(EntityEventCallback callback);
		VADONEDITOR_API void add_component_event_callback(ComponentEventCallback callback);

		VADONEDITOR_API const Scene* find_entity_scene(Vadon::ECS::EntityHandle entity) const;
	private:
		std::unordered_map<Vadon::Utilities::TypeUUID, Scene> m_scene_lookup;

		std::vector<EntityEventCallback> m_entity_callbacks;
		std::vector<ComponentEventCallback> m_component_callbacks;
	};
}
#endif