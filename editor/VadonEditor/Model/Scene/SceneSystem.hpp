#ifndef VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#define VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>
#include <VadonEditor/Model/Scene/Event.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <functional>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Scene;
	struct ResourceInfo;

	// TODO: implement public interface and private backend to avoid exposing functions (e.g entities signaling to the system)
	class SceneSystem
	{
	public:
		using EntityEventCallback = std::function<void(const EntityEvent&)>;
		using ComponentEventCallback = std::function<void(const ComponentEvent&)>;

		// TODO: API for creating scene local resource!
		// TODO2: API for opening a scene from a file (e.g selected in the asset browser)
		Scene* create_scene();
		Scene* get_scene(ResourceID scene_id);
		Scene* get_scene(Vadon::Scene::SceneHandle scene_handle);

		void open_scene(Scene* scene);
		void close_scene(Scene* scene);

		void remove_scene(Scene* scene);

		std::vector<ResourceInfo> get_scene_list() const;

		EntityID get_new_entity_id();

		VADONEDITOR_API void add_entity_event_callback(EntityEventCallback callback);
		VADONEDITOR_API void add_component_event_callback(ComponentEventCallback callback);

		// TODO: move to private backend!
		void entity_added(const Entity& entity);
		void entity_removed(const Entity& entity);

		void component_added(const Entity& owner, Vadon::Utilities::TypeID component_id);
		void component_edited(const Entity& owner, Vadon::Utilities::TypeID component_id); // TODO: also get the property that was edited!
		void component_removed(const Entity& owner, Vadon::Utilities::TypeID component_id);

		VADONEDITOR_API const Scene* find_entity_scene(Vadon::ECS::EntityHandle entity) const;
	private:
		SceneSystem(Core::Editor& editor);

		VADONEDITOR_API bool initialize();

		Scene* find_scene(Resource* resource);
		Scene* internal_get_scene(Resource* resource);
		void internal_remove_scene(Scene* scene);

		void dispatch_entity_event(const EntityEvent& event);
		void dispatch_component_event(const ComponentEvent& event);

		Core::Editor& m_editor;

		std::unordered_map<Resource*, Scene> m_scene_lookup;
		EntityID m_entity_id_counter;

		std::vector<EntityEventCallback> m_entity_callbacks;
		std::vector<ComponentEventCallback> m_component_callbacks;

		friend class ModelSystem;
	};
}
#endif