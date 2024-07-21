#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/World/World.hpp>

namespace Vadon::ECS
{
	World::World() = default;

	World::~World() = default;

	void World::remove_entity(EntityHandle entity_handle)
	{
		// Children will also be removed, so we remove their components
		const EntityList children = m_entity_manager.remove_entity(entity_handle);

		m_component_manager.remove_entity(entity_handle);
		m_component_manager.remove_entity_batch(children);
	}
}