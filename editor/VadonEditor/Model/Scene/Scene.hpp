#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Scene.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;
	class Resource;

	class Scene
	{
	public:
		Vadon::Scene::SceneHandle get_handle() const; 
		Vadon::Scene::ResourceInfo get_info() const;

		void set_path(const Vadon::Core::FileSystemPath& path);

		Entity* get_root() const { return m_root_entity; }
		bool is_open() const { return m_root_entity != nullptr; }

		bool save();
		bool load();

		bool is_loaded() const;

		Entity* add_new_entity(Entity& parent);
		Entity* instantiate_sub_scene(Scene* scene, Entity& parent);
		bool remove_entity(Entity& entity);

		Vadon::ECS::EntityHandle instantiate(bool is_sub_scene);

		// TODO: undo/redo?
		bool is_modified() const;
		void notify_modified();
		void clear_modified();
	private:
		Scene(Core::Editor& editor, Resource* scene_resource);

		bool initialize();
		void init_empty_scene();

		Entity* internal_add_new_entity(Entity* parent);
		Entity* internal_create_entity(Vadon::ECS::EntityHandle entity_handle, Entity* parent = nullptr);
		void internal_remove_entity(Entity* entity);

		bool is_owner_of_entity(Entity& entity) const;
		bool is_scene_dependent(Vadon::Scene::SceneHandle scene_handle) const;

		Entity* instantiate_scene_recursive(Vadon::ECS::EntityHandle entity_handle, Entity* parent = nullptr);

		void instantiate_from_root();
		void clear_contents();

		Core::Editor& m_editor;

		Resource* m_resource;

		Entity* m_root_entity;

		friend class SceneSystem;
	};
}
#endif