#ifndef VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#define VADONEDITOR_MODEL_SCENE_SCENESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Model/Scene/Entity.hpp>
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

	class SceneSystem
	{
	public:
		// FIXME: extend this so we can get info on what was modified!
		using EditCallback = std::function<void(Vadon::ECS::EntityHandle, Vadon::Utilities::TypeID)>;

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

		VADONEDITOR_API void register_edit_callback(EditCallback callback);
		void entity_edited(const Entity& entity, Vadon::Utilities::TypeID component_id);

		VADONEDITOR_API const Scene* find_entity_scene(Vadon::ECS::EntityHandle entity) const;
	private:
		SceneSystem(Core::Editor& editor);

		VADONEDITOR_API bool initialize();

		Scene* find_scene(Resource* resource);
		Scene* internal_get_scene(Resource* resource);
		void internal_remove_scene(Scene* scene);

		Core::Editor& m_editor;

		std::unordered_map<Resource*, Scene> m_scene_lookup;
		EntityID m_entity_id_counter;

		std::vector<EditCallback> m_edit_callbacks;

		friend class ModelSystem;
	};
}
#endif