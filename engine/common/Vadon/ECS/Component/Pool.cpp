#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/Pool.hpp>

#include <algorithm>

namespace Vadon::ECS
{
	void ComponentPoolInterface::dispatch_component_event(const ComponentEvent& event)
	{
		for (auto& current_callback : m_event_callbacks)
		{
			current_callback(event);
		}
	}

	std::optional<EntityList::const_iterator> DefaultComponentPoolBase::add_entity(EntityHandle entity)
	{
		auto entity_it = std::lower_bound(m_entity_lookup.begin(), m_entity_lookup.end(), entity);
		if (entity_it != m_entity_lookup.end())
		{
			if (entity == *entity_it)
			{
				// Entity already has this component
				// FIXME: do we allow one entity to own more than one of the same component?
				return entity_it;
			}
		}
		uint32_t component_offset = static_cast<uint32_t>(m_component_offsets.size());

		// Insert into lookup while keeping it ordered
		m_component_offsets.insert(m_component_offsets.begin() + std::distance(m_entity_lookup.begin(), entity_it), component_offset);
		m_entity_lookup.insert(entity_it, entity);

		return std::nullopt;
	}

	EntityList::const_iterator DefaultComponentPoolBase::find_entity(EntityHandle entity) const
	{
		auto entity_it = std::lower_bound(m_entity_lookup.begin(), m_entity_lookup.end(), entity);
		if (entity_it == m_entity_lookup.end())
		{
			return entity_it;
		}

		if (entity != (*entity_it))
		{
			return m_entity_lookup.end();
		}

		return entity_it;
	}

	void DefaultComponentPoolBase::remove_entity(EntityList::const_iterator entity_iterator)
	{
		const size_t entity_index = std::distance(m_entity_lookup.cbegin(), entity_iterator);

		// Find the Entity that pointed to the end of the component pool
		// FIXME: more efficient approach? Could use intrusive linked list to find last element
		auto remapped_offset_it = std::find(m_component_offsets.begin(), m_component_offsets.end(), static_cast<uint32_t>(m_entity_lookup.size()) - 1);

		const size_t remapped_entity_index = std::distance(m_component_offsets.begin(), remapped_offset_it);

		if (entity_index != remapped_entity_index)
		{
			// Remap entity that used to point to the end, now it should point to where the removed entity's component was
			*remapped_offset_it = m_component_offsets[entity_index];
		}

		// Remove the entity and its offset
		m_entity_lookup.erase(entity_iterator);
		m_component_offsets.erase(m_component_offsets.begin() + entity_index);
	}

	void DefaultComponentPoolBase::default_pool_clear()
	{
		m_entity_lookup.clear();
		m_component_offsets.clear();
	}
}