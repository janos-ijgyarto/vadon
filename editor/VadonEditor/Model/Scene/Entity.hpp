#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
namespace Vadon::ECS
{
	class World;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;
	using EntityList = std::vector<Entity*>;

	using EntityID = uint32_t;

	class Entity final
	{
	public:
		~Entity();

		EntityID get_id() const { return m_id; }

		Entity* get_parent() const { return m_parent; }
		const EntityList& get_children() const { return m_children; }

		bool has_visible_children() const; // FIXME: "visible" implies View, make this view-agnostic?

		const std::string& get_name() const { return m_name; }
		void set_name(std::string_view name);

		Scene* get_owning_scene() const { return m_owning_scene; }

		Scene* get_sub_scene() const { return m_sub_scene; }
		bool is_sub_scene() const { return get_sub_scene() != nullptr; }

		bool is_sub_scene_child() const { return (get_parent() != nullptr) && get_parent()->get_sub_scene(); }

		Vadon::ECS::EntityHandle get_handle() const { return m_entity_handle; }

		bool is_ancestor_of(const Entity& entity) const;
		std::string get_path() const;

		void add_child(Entity* entity, int32_t index = -1);
		void remove_child(Entity* entity);

		// TODO: "archetypes" which automatically add certain components?
		Entity* create_new_child();
		Entity* instantiate_child_scene(Vadon::Scene::SceneHandle scene_handle);

		bool add_component(Vadon::ECS::ComponentID type_id);
		void remove_component(Vadon::ECS::ComponentID type_id);

		Vadon::ECS::ComponentIDList get_component_types() const;
		Component get_component_data(Vadon::ECS::ComponentID component_id) const;

		Vadon::Utilities::TypeInfoList get_available_component_list() const;

		Vadon::Utilities::Variant get_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name) const;
		void edit_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Entity(Core::Editor& editor, Vadon::ECS::EntityHandle entity_handle, EntityID id, Scene* owning_scene);

		Vadon::ECS::World& get_ecs_world() const;
		void notify_modified();

		void internal_set_name(std::string_view name);
		void internal_remove_child(Entity* entity, bool reparent);
		void set_parent(Entity* parent);

		void enforce_unique_child_name(Entity* child, std::string_view name);

		Core::Editor& m_editor;

		EntityID m_id;
		Entity* m_parent;

		EntityList m_children;

		std::string m_name;

		Scene* m_owning_scene;
		Scene* m_sub_scene;

		Vadon::ECS::EntityHandle m_entity_handle;

		friend class Scene;
	};
}
#endif