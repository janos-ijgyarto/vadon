#ifndef VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#define VADONEDITOR_MODEL_SCENE_ENTITY_HPP
#include <VadonEditor/Model/Scene/Component.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <set>
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

		bool has_visible_children() const;

		std::string get_name() const { return m_name; }
		void set_name(std::string_view name);

		bool is_scene_child() const { return m_scene_child; }

		Vadon::ECS::EntityHandle get_handle() const { return m_entity_handle; }

		bool is_ancestor_of(const Entity& entity) const;
		std::string get_path() const;

		void add_child(Entity* entity, int32_t index = -1);
		void remove_child(Entity* entity);

		bool add_component(Vadon::ECS::ComponentID type_id);
		void remove_component(Vadon::ECS::ComponentID type_id);

		Vadon::ECS::ComponentIDList get_component_types() const;
		Component get_component_data(Vadon::ECS::ComponentID component_id) const;

		Vadon::Utilities::TypeInfoList get_available_component_list() const;

		Vadon::Utilities::Variant get_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name);
		void edit_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Entity(Core::Editor& editor, Vadon::ECS::EntityHandle entity_handle, EntityID id, Entity* parent = nullptr);
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

		// FIXME: use reference to "scene" object so this can also be displayed in the View!
		bool m_scene_child = false;

		Vadon::ECS::EntityHandle m_entity_handle;

		friend class SceneTree;
	};
}
#endif