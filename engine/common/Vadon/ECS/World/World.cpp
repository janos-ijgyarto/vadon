#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/World/World.hpp>

namespace Vadon::ECS
{
	World::World() = default;

	World::~World() = default;

	void World::remove_pending_entities()
	{
		if (m_entity_manager.m_pending_remove_list.empty() == true)
		{
			return;
		}

		// Remove all the components
		m_component_manager.remove_entity_batch(m_entity_manager.m_pending_remove_list);

		// Remove from entity manager
		m_entity_manager.remove_pending_entities();
	}

	void World::clear()
	{
		m_entity_manager.clear();
		m_component_manager.clear();
	}
}