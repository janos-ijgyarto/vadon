#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;

	// FIXME: could this be merged with the Editor Resource object?
	class Scene
	{
	public:
		Vadon::Scene::SceneHandle get_handle() const { return m_handle; }
		const Vadon::Scene::ResourceInfo& get_info() const { return m_info; }

		void set_path(const Vadon::Scene::ResourcePath& path);

		Entity* get_root() const { return m_root_entity; }
		bool is_open() const { return m_root_entity != nullptr; }

		bool save();
		bool load();

		Entity* add_new_entity(Entity& parent);
		Entity* instantiate_sub_scene(Vadon::Scene::SceneHandle scene_handle, Entity& parent);
		bool remove_entity(Entity& entity);

		Vadon::ECS::EntityHandle instantiate(bool is_sub_scene);

		// TODO: undo/redo?
		bool is_modified() const { return m_modified; }
		void notify_modified() { m_modified = true; }
	private:
		Scene(Core::Editor& editor, Vadon::Scene::SceneHandle scene_handle);

		bool initialize();
		void init_empty_scene();

		void update_info();
		void clear_modified() { m_modified = false; }

		Entity* internal_add_new_entity(Entity* parent);
		Entity* internal_create_entity(Vadon::ECS::EntityHandle entity_handle, Entity* parent = nullptr);
		void internal_remove_entity(Entity* entity);

		bool is_owner_of_entity(Entity& entity) const;

		Entity* instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent = nullptr);

		void instantiate_from_root();
		void clear_contents();

		Core::Editor& m_editor;

		Vadon::Scene::SceneHandle m_handle;
		Vadon::Scene::ResourceInfo m_info;

		Entity* m_root_entity;
		bool m_modified;

		friend class SceneSystem;
	};
}
#endif