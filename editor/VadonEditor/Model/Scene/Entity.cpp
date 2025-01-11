#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <format>

namespace
{
	// FIXME: implement proper filtering API!
	bool is_component_type_excluded(Vadon::ECS::ComponentID component_id)
	{
		// Exclude scene component
		if (component_id == Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::SceneComponent>())
		{
			return true;
		}

		return false;
	}
}

namespace VadonEditor::Model
{
	Entity::~Entity()
	{
		// Remove self from parent
		if (m_parent != nullptr)
		{
			m_parent->internal_remove_child(this, true);
		}

		for (Entity* current_child : m_children)
		{
			// Unset parent in children so they don't redundantly remove themselves
			current_child->m_parent = nullptr;
			delete current_child;
		}
	}

	bool Entity::has_visible_children() const
	{
		for (const Entity* current_child : m_children)
		{
			if (current_child->is_sub_scene_child() == false)
			{
				return true;
			}
		}

		return false;
	}

	void Entity::set_name(std::string_view name)
	{
		if (m_parent != nullptr)
		{
			m_parent->enforce_unique_child_name(this, name);
		}
		else
		{
			internal_set_name(name);
		}
	}

	bool Entity::is_ancestor_of(const Entity& entity) const
	{
		const Entity* parent = entity.get_parent();
		while (parent != nullptr)
		{
			if (parent == this)
			{
				return true;
			}

			parent = parent->get_parent();
		}

		return false;
	}

	std::string Entity::get_path() const
	{
		std::string path;

		Entity* parent = get_parent();
		while (parent)
		{
			path = parent->get_name() + "/" + path;
			parent = parent->get_parent();
		}

		if (path.empty() == false)
		{
			// Trim the trailing slash
			path.pop_back();
		}
		else
		{
			path = ".";
		}

		return path;
	}

	void Entity::add_child(Entity* entity, int32_t index)
	{
		if (entity->is_ancestor_of(*this) == true)
		{
			// TODO: throw error?
			return;
		}

		if (entity->m_parent != nullptr)
		{
			entity->m_parent->internal_remove_child(entity, true);
		}

		if ((index >= 0) && (index < static_cast<int32_t>(m_children.size())))
		{
			m_children.insert(m_children.begin() + index, entity);
		}
		else
		{
			// Add to end
			m_children.push_back(entity);
		}

		enforce_unique_child_name(entity, entity->get_name());

		entity->set_parent(this);

		Vadon::ECS::World& world = get_ecs_world();
		world.get_entity_manager().add_child_entity(m_entity_handle, entity->m_entity_handle);
	}

	void Entity::remove_child(Entity* entity)
	{
		internal_remove_child(entity, false);

		Vadon::ECS::World& world = get_ecs_world();
		world.get_entity_manager().remove_child_entity(m_entity_handle, entity->m_entity_handle);
	}

	Entity* Entity::create_new_child()
	{
		return m_owning_scene->add_new_entity(*this);
	}

	Entity* Entity::instantiate_child_scene(Scene* scene)
	{
		return m_owning_scene->instantiate_sub_scene(scene, *this);
	}

	bool Entity::add_component(Vadon::ECS::ComponentID type_id)
	{
		if (is_sub_scene() == true)
		{
			m_editor.get_engine_core().log_error("Cannot modify component structure of sub-scene Entity!\n");
			return false;
		}

		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		if (component_manager.has_component(m_entity_handle, type_id) == true)
		{
			return false;
		}

		if (world.get_component_manager().add_component(m_entity_handle, type_id) == nullptr)
		{
			return false;
		}

		notify_modified();
		return true;
	}

	void Entity::remove_component(Vadon::ECS::ComponentID type_id)
	{
		if (is_sub_scene() == true)
		{
			m_editor.get_engine_core().log_error("Cannot modify component structure of sub-scene Entity!\n");
			return;
		}

		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();
		
		component_manager.remove_component(m_entity_handle, type_id);
		notify_modified();
	}

	Vadon::ECS::ComponentIDList Entity::get_component_types() const
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		Vadon::ECS::ComponentIDList component_ids = component_manager.get_component_list(get_handle());

		// Remove all excluded IDs
		component_ids.erase(std::remove_if(component_ids.begin(), component_ids.end(),
			+[](Vadon::ECS::ComponentID current_id) -> bool { return is_component_type_excluded(current_id); }), component_ids.end());

		return component_ids;
	}

	Component Entity::get_component_data(Vadon::ECS::ComponentID component_id) const
	{
		Vadon::ECS::World& ecs_world = m_editor.get_system<Model::ModelSystem>().get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		Component component;
		component.name = Vadon::Utilities::TypeRegistry::get_type_info(component_id).name;

		void* component_ptr = component_manager.get_component(get_handle(), component_id);

		component.properties = Vadon::Utilities::TypeRegistry::get_properties(component_ptr, component_id);

		return component;
	}

	Vadon::Utilities::TypeInfoList Entity::get_available_component_list() const
	{
		Vadon::Utilities::TypeInfoList component_type_list;

		// Get only the component types which haven't been added yet
		// Also exclude certain utility components (e.g Scene Component)
		// FIXME: implement "flagging" system to decide which components to show?
		const Vadon::ECS::ComponentIDList instance_component_ids = get_component_types();
		const Vadon::ECS::ComponentIDList all_component_ids = Vadon::ECS::ComponentRegistry::get_component_types();

		for (Vadon::ECS::ComponentID current_type_id : all_component_ids)
		{
			if (std::find(instance_component_ids.begin(), instance_component_ids.end(), current_type_id) != instance_component_ids.end())
			{
				// Component already added
				continue;
			}

			if (is_component_type_excluded(current_type_id) == true)
			{
				continue;
			}

			component_type_list.push_back(Vadon::Utilities::TypeRegistry::get_type_info(current_type_id));
		}

		return component_type_list;
	}

	Vadon::Utilities::Variant Entity::get_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name) const
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		// FIXME: wrap this in the ECS?
		void* component = component_manager.get_component(m_entity_handle, component_type_id);
		return Vadon::Utilities::TypeRegistry::get_property(component, component_type_id, property_name);
	}

	void Entity::edit_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		// FIXME: wrap this in the ECS?
		void* component = component_manager.get_component(m_entity_handle, component_type_id);
		Vadon::Utilities::TypeRegistry::set_property(component, component_type_id, property_name, value);

		m_editor.get_system<ModelSystem>().get_scene_system().entity_edited(*this, component_type_id);
		notify_modified();
	}

	Entity::Entity(Core::Editor& editor, Vadon::ECS::EntityHandle entity_handle, EntityID id, Scene* owning_scene)
		: m_editor(editor)
		, m_id(id)
		, m_parent(nullptr)
		, m_entity_handle(entity_handle)
		, m_owning_scene(owning_scene)
		, m_sub_scene(nullptr)
	{
	}

	Vadon::ECS::World& Entity::get_ecs_world() const
	{
		return m_editor.get_system<ModelSystem>().get_ecs_world();
	}

	void Entity::notify_modified()
	{
		m_owning_scene->notify_modified();
	}

	void Entity::internal_set_name(std::string_view name)
	{
		m_name = name;

		Vadon::ECS::World& world = get_ecs_world();
		world.get_entity_manager().set_entity_name(m_entity_handle, m_name);

		notify_modified();
	}

	void Entity::internal_remove_child(Entity* entity, bool reparent)
	{
		auto child_it = std::find(m_children.begin(), m_children.end(), entity);
		if (child_it != m_children.end())
		{
			m_children.erase(child_it);
			if (reparent == false)
			{
				entity->set_parent(nullptr);
			}
		}
	}

	void Entity::set_parent(Entity* parent)
	{
		m_parent = parent;
	}

	void Entity::enforce_unique_child_name(Entity* child, std::string_view name)
	{
		auto child_name_search = [this, &child](std::string_view unique_name)
			{
				for (Entity* current_child : m_children)
				{
					if ((current_child != child) && (current_child->m_name == unique_name))
					{
						return true;
					}
				}

				return false;
			};

		size_t attempt_count = 1;
		std::string unique_child_name(name);
		while (child_name_search(unique_child_name) == true)
		{
			++attempt_count;
			unique_child_name = std::format("{}_{}", name, attempt_count);
		}

		child->internal_set_name(unique_child_name);
	}
}