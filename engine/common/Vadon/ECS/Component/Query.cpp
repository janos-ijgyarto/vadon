#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/Query.hpp>

namespace Vadon::ECS
{
	bool ComponentQueryBase::IteratorBase::is_valid() const
	{
		return m_offset < m_query.m_entities.size();
	}

	EntityHandle ComponentQueryBase::IteratorBase::get_entity() const
	{
		return m_query.m_entities[m_offset];
	}

	void ComponentQueryBase::IteratorBase::next()
	{
		++m_offset;
		advance();
	}

	ComponentQueryBase::IteratorBase::IteratorBase(ComponentQueryBase& query)
		: m_query(query)
		, m_offset(0)
	{
		advance();
	}

	void ComponentQueryBase::IteratorBase::advance()
	{
		while (is_valid() == true)
		{
			if (m_query.check_components(m_offset) == true)
			{
				break;
			}
			++m_offset;
		}
	}

	void ComponentQueryBase::initialize(std::span<ComponentInfo> component_info)
	{
		m_component_info = component_info;

		// Start by sorting based on how many entities are available, prioritizing required pools
		struct PoolSortInfo
		{
			size_t index;
			uint32_t active_count;
			bool required;

			bool operator<(const PoolSortInfo& other) const
			{
				if (required == other.required)
				{
					if (required == true)
					{
						// With required pools, we want the "narrowest"
						return active_count < other.active_count;
					}
					else
					{
						// With optional pools, we want the "widest" (in case our query only has optional components)
						return other.active_count < active_count;
					}
				}
				// Required components should be considered first
				return other.required < required;
			}
		};

		std::vector<PoolSortInfo> pool_sort_vec;
		{
			size_t pool_index = 0;
			for (const ComponentInfo& current_component_info : m_component_info)
			{
				if ((current_component_info.pool == nullptr) && (current_component_info.required == true))
				{
					// Required component pool not present
					return;
				}

				const uint32_t active_count = (current_component_info.pool != nullptr) ? current_component_info.pool->get_active_count() : 0;

				pool_sort_vec.emplace_back(pool_index, active_count, current_component_info.required);
				++pool_index;
			}
		}

		std::sort(pool_sort_vec.begin(), pool_sort_vec.end());

		// Use the most narrow pool as the "starting point"
		ComponentPoolInterface* pool = m_component_info[pool_sort_vec.front().index].pool;
		if (pool != nullptr)
		{
			m_entities = pool->get_entities();
		}
	}

	bool ComponentQueryBase::check_components(size_t index)
	{
		EntityHandle entity = m_entities[index];

		bool has_any = false; // Check whether at least one of the queried components is present (in case they were all optional)
		for (const ComponentInfo& current_component_info : m_component_info)
		{
			if (current_component_info.pool != nullptr)
			{
				const bool has_component = current_component_info.pool->has_component(entity);
				if ((current_component_info.required == true) && (has_component == false))
				{
					// Required component not present
					return false;
				}
				has_any |= has_component;
			}
		}

		return has_any;
	}
}