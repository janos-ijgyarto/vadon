#ifndef VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <VadonEditor/Model/Scene/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>

#include <functional>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;

	class SceneTree
	{
	public:
		// FIXME: extend this so we can get info on what was modified!
		using EditCallback = std::function<void(Vadon::ECS::EntityHandle, Vadon::Utilities::TypeID)>;

		Vadon::Scene::ResourceHandle get_current_scene() const { return m_current_scene.scene; }
		Entity* get_root() const { return m_current_scene.root_entity; }

		// TODO: "archetypes" which automatically add certain components?
		// FIXME: use object pool so the raw pointers are not exposed?
		void add_entity(Entity* parent = nullptr);
		void remove_entity(Entity* entity);

		bool instantiate_sub_scene(Entity* parent, Vadon::Scene::ResourceHandle scene_handle);

		bool new_scene(std::string_view name);
		bool save_scene();
		bool load_scene(Vadon::Scene::ResourceHandle scene_handle);

		// TODO: undo/redo?
		void notify_scene_modified() { m_current_scene.is_modified = true; }
		bool is_scene_modified() const { return m_current_scene.is_modified; }

		Scene get_scene_info(Vadon::Scene::ResourceHandle scene_handle) const;
		SceneList get_scene_list() const;

		VADONEDITOR_API void register_edit_callback(EditCallback callback);
		void entity_edited(Vadon::ECS::EntityHandle entity, Vadon::Utilities::TypeID component_id);
	private:
		struct SceneData
		{
			Vadon::Scene::ResourceHandle scene;
			Entity* root_entity = nullptr;
			bool is_modified = false;
		};

		SceneTree(Core::Editor& editor);

		bool initialize();
		Entity* instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent);
		
		void clear_scene();
		void scene_changes_cleared() { m_current_scene.is_modified = false; }

		void internal_remove_entity(Entity* entity);

		Core::Editor& m_editor;

		SceneData m_current_scene;
		uint32_t m_entity_id_counter = 0;

		std::vector<EditCallback> m_edit_callbacks;

		friend class ModelSystem;
	};
}
#endif