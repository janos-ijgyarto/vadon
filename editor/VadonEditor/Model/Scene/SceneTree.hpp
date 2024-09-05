#ifndef VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENETREE_HPP
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <VadonEditor/Model/Scene/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
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
		Vadon::Scene::ResourceHandle get_current_scene() const { return m_current_scene.scene; }
		Entity* get_root() const { return m_current_scene.root_entity; }

		// TODO: "archetypes" which automatically add certain components?
		// FIXME: use object pool so the raw pointers are not exposed?
		void add_entity(Entity* parent = nullptr);
		void remove_entity(Entity* entity);

		bool instantiate_sub_scene(Entity* parent, Vadon::Scene::ResourceID scene_id);

		bool new_scene(std::string_view name);
		bool save_scene();
		bool load_scene(Vadon::Scene::ResourceID scene_id);

		// TODO: undo/redo?
		void notify_scene_modified() { m_current_scene.is_modified = true; }
		bool is_scene_modified() const { return m_current_scene.is_modified; }

		Scene get_scene_info(Vadon::Scene::ResourceHandle scene_handle) const;
		SceneList get_scene_list() const;
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

		Core::Editor& m_editor;

		SceneData m_current_scene;
		uint32_t m_entity_id_counter = 0;

		friend class ModelSystem;
	};
}
#endif