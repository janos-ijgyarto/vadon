#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Container/DataBuffer/AllocationManager.hpp>

namespace Vadon::Utilities
{
	DataAllocationManager::DataAllocationManager()
		: m_allocation_counter(0)
		, m_min_allocation_id(-1)
	{

	}

	void DataAllocationManager::reset()
	{
		m_allocations.clear();

		// Update the min ID
		m_min_allocation_id = m_allocation_counter++;
	}

	DataAllocation DataAllocationManager::allocate(int32_t count)
	{
		if (!m_allocations.empty())
		{
			return allocate_internal(find_slot(count), count);
		}
		else
		{
			// Buffer is empty, simply use first slot
			DataAllocation& new_allocation = m_allocations.emplace_back();
			new_allocation.id = m_allocation_counter++;

			new_allocation.range.count = count;
			new_allocation.range.offset = 0;

			return new_allocation;
		}
	}

	void DataAllocationManager::release(const DataAllocation& allocation)
	{
		if (allocation.id <= m_min_allocation_id)
		{
			return;
		}

		// Use binary search (entries are sorted by offset)
		auto removed_allocation_it = std::lower_bound(m_allocations.begin(), m_allocations.end(), allocation,
			[](const DataAllocation& current_allocation, const DataAllocation& released_allocation)
			{
				return (current_allocation.range.offset < released_allocation.range.offset);
			}
		);

		// Make sure ID matches
		if ((removed_allocation_it != m_allocations.end()) && (removed_allocation_it->id == allocation.id))
		{
			m_allocations.erase(removed_allocation_it);
		}
		// TODO: error if entry not found?
	}

	int32_t DataAllocationManager::get_size() const
	{
		if (!m_allocations.empty())
		{
			const DataAllocation& last_allocation = m_allocations.back();
			return (last_allocation.range.offset + last_allocation.range.count);
		}

		return 0;
	}

	DataAllocationManager::AllocationVector::iterator DataAllocationManager::find_slot(int32_t count)
	{
		int32_t prev_allocation_end = 0;

		for (auto allocation_it = m_allocations.begin(); allocation_it != m_allocations.end(); ++allocation_it)
		{
			const DataAllocation& current_allocation = *allocation_it;
			const int32_t current_slot_size = current_allocation.range.offset - prev_allocation_end;
			if (current_slot_size >= count)
			{
				// Found a viable slot
				// TODO: some kind of optimization to make better use of "holes"?
				return allocation_it;
			}

			prev_allocation_end = current_allocation.range.offset + current_allocation.range.count;
		}

		return m_allocations.end();
	}

	DataAllocation DataAllocationManager::allocate_internal(AllocationVector::iterator slot, int32_t count)
	{
		DataAllocation new_allocation;
		new_allocation.id = m_allocation_counter++;

		new_allocation.range.count = count;

		if (slot == m_allocations.begin())
		{
			new_allocation.range.offset = 0;
		}
		else
		{
			const DataAllocation& prev_allocation = *(slot - 1);
			new_allocation.range.offset = prev_allocation.range.offset + prev_allocation.range.count;
		}

		m_allocations.insert(slot, new_allocation);
		return new_allocation;
	}
}
