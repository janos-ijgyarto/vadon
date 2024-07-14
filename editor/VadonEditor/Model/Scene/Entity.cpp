#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Model/ModelSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <format>

namespace VadonEditor::Model
{
	Entity::~Entity()
	{
		if (m_parent != nullptr)
		{
			m_parent->internal_remove_child(this, true);
		}

		if (m_entity_handle.is_valid() == true)
		{
			// Remove ECS entity, this will also remove all of its children
			Vadon::ECS::World& world = get_ecs_world();
			world.remove_entity(m_entity_handle);
		}

		for (Entity* current_child : m_children)
		{
			// Unset parent in children so they don't redundantly remove themselves
			current_child->m_parent = nullptr;

			// Invalidate entity handle in child so we don't try to redundantly delete entities
			current_child->m_entity_handle.invalidate();
			delete current_child;
		}
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

	bool Entity::add_component(Vadon::ECS::ComponentID type_id)
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		if (component_manager.has_component(m_entity_handle, type_id) == true)
		{
			return false;
		}

		return world.get_component_manager().add_component(m_entity_handle, type_id) != nullptr;
	}

	void Entity::remove_component(Vadon::ECS::ComponentID type_id)
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();
		
		component_manager.remove_component(m_entity_handle, type_id);
	}

	Vadon::Utilities::Variant Entity::get_component_property(Vadon::ECS::ComponentID component_type_id, std::string_view property_name)
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
	}

	Vadon::ECS::ComponentIDList Entity::get_component_types()
	{
		Vadon::ECS::World& world = get_ecs_world();
		Vadon::ECS::ComponentManager& component_manager = world.get_component_manager();

		return component_manager.get_component_list(m_entity_handle);
	}

	Entity::Entity(Core::Editor& editor, Vadon::ECS::EntityHandle entity_handle, Entity* parent)
		: m_editor(editor)
		, m_parent(parent)
		, m_entity_handle(entity_handle)
	{
	}

	Vadon::ECS::World& Entity::get_ecs_world()
	{
		return m_editor.get_system<ModelSystem>().get_ecs_world();
	}

	void Entity::internal_set_name(std::string_view name)
	{
		m_name = name;

		Vadon::ECS::World& world = get_ecs_world();
		world.get_entity_manager().set_entity_name(m_entity_handle, m_name);
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
			unique_child_name = std::format("{}_{}", child->get_name(), attempt_count);
		}

		child->internal_set_name(unique_child_name);
	}
}