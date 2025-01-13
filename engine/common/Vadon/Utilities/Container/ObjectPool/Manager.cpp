#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Manager.hpp>

namespace Vadon::Utilities
{
	ObjectPoolManager::ObjectPoolManager()
		: m_first_free_index(0)
		, m_generation(1)
	{
	}

	void ObjectPoolManager::reset()
	{
		m_first_free_index = 0;

		uint32_t next_free_index = 1;
		for (Entry& current_entry : m_entries)
		{
			// Reset active flag and generation
			current_entry.active = false;
			current_entry.generation = 1;

			// Reset free list
			current_entry.next_free_index = next_free_index++;
			++current_entry.generation;
		}

		// Increment pool generation so all dangling handles become invalid
		++m_generation;
	}

	ObjectPoolHandle ObjectPoolManager::add()
	{
		ObjectPoolHandle new_handle;
		new_handle.index = m_first_free_index;

		if (m_first_free_index == static_cast<uint32_t>(m_entries.size()))
		{
			// Add new entry
			Entry& new_entry = m_entries.emplace_back();

			++m_first_free_index;
			new_entry.next_free_index = m_first_free_index;
		}

		// Initialize handle based on pool entry
		Entry& selected_entry = m_entries[new_handle.index];
		selected_entry.active = true;

		m_first_free_index = selected_entry.next_free_index;
		new_handle.generation = selected_entry.generation;
		new_handle.pool_generation = m_generation;

		return new_handle;

	}

	bool ObjectPoolManager::validate(const ObjectPoolHandle& handle) const
	{
		if ((handle.is_valid() == false) || (handle.pool_generation != m_generation))
		{
			return false;
		}

		if (handle.index > static_cast<uint32_t>(m_entries.size()))
		{
			return false;
		}

		const Entry& selected_entry = m_entries[handle.index];
		return (selected_entry.active && (selected_entry.generation == handle.generation));
	}

	void ObjectPoolManager::remove(const ObjectPoolHandle& handle)
	{
		VADON_ASSERT(validate(handle) == true, "Tried to remove object with invalid handle!");

		Entry& released_entry = m_entries[handle.index];
		released_entry.active = false;

		++released_entry.generation; // Increment generation to ensure old handles are invalid

		// Update free list
		released_entry.next_free_index = m_first_free_index;
		m_first_free_index = handle.index;
	}

	ObjectPoolHandle ObjectPoolManager::Iterator::get_handle() const
	{
		ObjectPoolHandle handle;
		if (is_valid() == true)
		{
			handle.index = static_cast<uint32_t>(std::distance(m_manager.m_entries.cbegin(), m_entry_it));
			handle.generation = m_entry_it->generation;
			handle.pool_generation = m_manager.m_generation;
		}
		return handle;
	}
}