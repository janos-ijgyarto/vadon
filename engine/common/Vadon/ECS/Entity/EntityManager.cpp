#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Entity/EntityManager.hpp>

#include <Vadon/Core/Logger.hpp>

namespace Vadon::ECS
{
	EntityHandle EntityManager::create_entity()
	{
		EntityHandle new_entity_handle = m_entity_pool.add();
		// TODO: anything else?
		return new_entity_handle;
	}

	std::string EntityManager::get_entity_name(EntityHandle entity_handle) const
	{
		const EntityData& entity_data = m_entity_pool.get(entity_handle);
		return entity_data.name;
	}

	void EntityManager::set_entity_name(EntityHandle entity_handle, std::string_view name)
	{
		EntityData& entity_data = m_entity_pool.get(entity_handle);
		entity_data.name = name;
	}

	EntityHandle EntityManager::get_entity_parent(EntityHandle entity) const
	{
		const EntityData& entity_data = m_entity_pool.get(entity);
		return entity_data.parent;
	}

	EntityHandle EntityManager::get_entity_root(EntityHandle entity) const
	{
		VADON_ASSERT(entity.is_valid(), "Invalid entity handle!");
		Vadon::ECS::EntityHandle root_entity = entity;
		while (true)
		{
			const EntityData& entity_data = m_entity_pool.get(root_entity);
			Vadon::ECS::EntityHandle parent = entity_data.parent;
			if (parent.is_valid() == false)
			{
				break;
			}

			root_entity = parent;
		}

		return root_entity;
	}

	void EntityManager::set_entity_parent(EntityHandle entity, EntityHandle parent)
	{
		EntityData& entity_data = m_entity_pool.get(entity);
		if (entity_data.parent == parent)
		{
			return;
		}

		// Make sure we aren't trying to make a cycle
		if (is_ancestor(parent, entity) == true)
		{
			Vadon::Core::Logger::log_error("Entity manager: detected cyclical parent-child relationship between entities!\n");
			return;
		}

		if (entity_data.parent.is_valid() == true)
		{
			// Remove from previous parent
			internal_remove_child_entity(entity_data.parent, entity);
		}

		// Add to new parent
		entity_data.parent = parent;
		if (parent.is_valid() == true)
		{
			internal_add_child_entity(parent, entity);
		}
	}

	void EntityManager::remove_child_entity(EntityHandle parent, EntityHandle child)
	{
		// Make sure the parent actually contained this child
		const EntityData& child_data = m_entity_pool.get(child);
		if (child_data.parent == parent)
		{
			set_entity_parent(child, EntityHandle());
		}
	}

	bool EntityManager::is_ancestor(EntityHandle entity, EntityHandle ancestor) const
	{
		if (ancestor.is_valid() == false)
		{
			return false;
		}

		const EntityData& entity_data = m_entity_pool.get(entity);
		EntityHandle current_ancestor = entity_data.parent;
		while (current_ancestor.is_valid() == true)
		{
			if (current_ancestor == ancestor)
			{
				return true;
			}

			const EntityData& ancestor_data = m_entity_pool.get(current_ancestor);
			current_ancestor = ancestor_data.parent;
		}

		return false;
	}

	EntityList EntityManager::get_children(EntityHandle entity, bool recursive) const
	{
		const EntityData& entity_data = m_entity_pool.get(entity);
		std::vector<EntityHandle> children = entity_data.children;

		if (recursive == true)
		{
			size_t child_index = 0;
			while (child_index < children.size())
			{
				const EntityHandle& current_child_handle = children[child_index];
				const EntityData& child_data = m_entity_pool.get(current_child_handle);
				children.insert(children.end(), child_data.children.begin(), child_data.children.end());
				++child_index;
			}
		}

		return children;
	}

	EntityList EntityManager::get_active_entities() const
	{
		EntityList active_entities;
		for (const EntityPool::ConstObjectPair& entity_pair : m_entity_pool)
		{
			active_entities.push_back(entity_pair.first);
		}

		return active_entities;
	}

	EntityManager::EntityManager() = default;

	EntityList EntityManager::remove_entity(EntityHandle entity_handle)
	{
		// Remove from parent
		const EntityData& entity_data = m_entity_pool.get(entity_handle);
		if (entity_data.parent.is_valid() == true)
		{
			internal_remove_child_entity(entity_data.parent, entity_handle);
		}

		// Need to remove all the children as well
		const EntityList children = EntityManager::get_children(entity_handle, true);
		for (const EntityHandle current_child : children)
		{
			m_entity_pool.remove(current_child);
		}

		// Remove the original entity
		m_entity_pool.remove(entity_handle);

		// Return child list (used by World to clear components)
		return children;
	}

	void EntityManager::internal_add_child_entity(EntityHandle parent, EntityHandle child)
	{
		EntityData& parent_data = m_entity_pool.get(parent);
		parent_data.children.push_back(child);
	}

	void EntityManager::internal_remove_child_entity(EntityHandle parent, EntityHandle child)
	{
		EntityData& parent_data = m_entity_pool.get(parent);
		parent_data.children.erase(std::remove(parent_data.children.begin(), parent_data.children.end(), child), parent_data.children.end());
	}

	void EntityManager::clear()
	{
		m_entity_pool.reset();
	}
}