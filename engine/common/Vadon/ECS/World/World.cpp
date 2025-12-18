#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/World/World.hpp>

namespace Vadon::ECS
{
	World::World() = default;

	World::~World() = default;

	void World::remove_entity(EntityHandle entity)
	{
		// Remove root from its parent so it's not affected
		{
			EntityHandle parent_entity = m_entity_manager.get_entity_parent(entity);
			if (parent_entity.is_valid() == true)
			{
				m_entity_manager.remove_child_entity(parent_entity, entity);
			}
		}

		const EntityList entity_subtree = m_entity_manager.get_children(entity, true);

		// Remove all the components
		m_component_manager.remove_entity_batch(entity_subtree);
		m_component_manager.remove_entity(entity);

		// Remove all from entity manager (no need to manage parent-child connections)
		for (EntityHandle current_entity : entity_subtree)
		{
			m_entity_manager.internal_remove_entity(current_entity);
		}
		m_entity_manager.internal_remove_entity(entity);
	}

	void World::clear()
	{
		m_entity_manager.clear();
		m_component_manager.clear();
	}
}