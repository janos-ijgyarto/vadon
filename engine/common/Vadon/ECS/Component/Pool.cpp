#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/Pool.hpp>

#include <algorithm>

namespace Vadon::ECS
{
	bool DefaultComponentPoolBase::add_entity(EntityHandle entity)
	{
		if (entity.handle.index >= m_sparse_lookup.size())
		{
			// New entity, need to extend lookup
			m_sparse_lookup.insert(m_sparse_lookup.end(), (entity.handle.index - m_sparse_lookup.size()) + 1, c_invalid_component_index);
		}

		const uint32_t prev_index = m_sparse_lookup[entity.handle.index];
		if (prev_index != c_invalid_component_index)
		{
			// If there is a valid offset at this entry, then this Entity was already registered, otherwise something went very wrong
			VADON_ASSERT(m_reverse_lookup[prev_index] == entity, "Invalid index in component pool!");
			return false;
		}

		// New component is added at the back, so we push the entity to the back as well (for reverse lookup)
		m_sparse_lookup[entity.handle.index] = static_cast<uint32_t>(m_reverse_lookup.size());
		m_reverse_lookup.push_back(entity);

		return true;
	}

	uint32_t DefaultComponentPoolBase::get_component_index(EntityHandle entity) const
	{
		if (entity.handle.index >= m_sparse_lookup.size())
		{
			return c_invalid_component_index;
		}

		return m_sparse_lookup[entity.handle.index];
	}

	uint32_t DefaultComponentPoolBase::remove_entity(EntityHandle entity)
	{
		if (entity.handle.index >= m_sparse_lookup.size())
		{
			// Entity doesn't have a component in this pool
			return c_invalid_component_index;
		}

		const uint32_t component_index = m_sparse_lookup[entity.handle.index];
		if (component_index == c_invalid_component_index)
		{
			// Entity doesn't have a component in this pool
			return c_invalid_component_index;
		}

		VADON_ASSERT(m_reverse_lookup[component_index] == entity, "Invalid handle in component pool!");

		if (component_index != (m_reverse_lookup.size() - 1))
		{
			// Make sure we update the entry in the sparse lookup for the component we moved
			const EntityHandle moved_entry = m_reverse_lookup.back();
			m_reverse_lookup[component_index] = moved_entry;

			m_sparse_lookup[moved_entry.handle.index] = component_index;
		}
		m_reverse_lookup.pop_back();

		// TODO: shrink if possible?
		m_sparse_lookup[entity.handle.index] = c_invalid_component_index;
		return component_index;
	}

	void DefaultComponentPoolBase::default_pool_clear()
	{
		m_reverse_lookup.clear();
		m_sparse_lookup.clear();
	}

	bool TagPoolBase::add_entity(EntityHandle entity)
	{
		return m_entity_lookup.insert(entity.handle.to_uint()).second;
	}

	bool TagPoolBase::has_entity(EntityHandle entity) const
	{
		return m_entity_lookup.find(entity.handle.to_uint()) != m_entity_lookup.end();
	}

	void TagPoolBase::remove_entity(EntityHandle entity)
	{
		m_entity_lookup.erase(entity.handle.to_uint());
	}

	void TagPoolBase::tag_pool_clear()
	{
		m_entity_lookup.clear();
	}

	EntityList TagPoolBase::get_tagged_entity_list() const
	{
		EntityList entity_list;
		entity_list.reserve(m_entity_lookup.size());

		for (uint64_t current_entity : m_entity_lookup)
		{
			entity_list.push_back(EntityHandle{ .handle = Vadon::Utilities::ObjectPoolHandle().from_uint(current_entity) });
		}

		return entity_list;
	}
}