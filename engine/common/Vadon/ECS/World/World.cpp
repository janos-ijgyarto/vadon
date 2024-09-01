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

	void World::clear()
	{
		dispatch_event(WorldEvent{ .event_type = WorldEventType::CLEARED });

		m_entity_manager.clear();
		m_component_manager.clear();
	}

	void World::register_event_callback(WorldEventCallback callback)
	{
		m_event_callbacks.push_back(callback);
	}

	void World::dispatch_event(const WorldEvent& event) const
	{
		for (const WorldEventCallback& current_callback : m_event_callbacks)
		{
			current_callback(event);
		}
	}
}