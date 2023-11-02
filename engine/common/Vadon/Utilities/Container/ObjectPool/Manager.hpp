#ifndef VADON_UTILITIES_CONTAINER_OBJECTPOOL_MANAGER_HPP
#define VADON_UTILITIES_CONTAINER_OBJECTPOOL_MANAGER_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <vector>
namespace Vadon::Utilities
{
	class VADONCOMMON_API ObjectPoolManager
	{
	public:
		ObjectPoolManager();

		void reset();

		ObjectPoolHandle add();
		bool validate(const ObjectPoolHandle& handle) const;
		void remove(const ObjectPoolHandle& handle);
	private:
		struct Entry
		{
			uint32_t next_free_index = 0;
			uint16_t generation = 1;
			bool active = false;
		};

		using EntryVector = std::vector<Entry>;
		EntryVector m_entries;

		uint32_t m_first_free_index;
		uint16_t m_generation;
	public:
		class VADONCOMMON_API Iterator
		{
		public:
			Iterator(ObjectPoolManager& manager, EntryVector::iterator entry_it)
				: m_manager(manager)
				, m_entry_it(entry_it)
			{
			}

			Iterator& operator++()
			{
				++m_entry_it;
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator temp = *this;
				++(*this);

				return temp;
			}

			bool operator==(const Iterator& rhs) const { return m_entry_it == rhs.m_entry_it; }
			bool operator!=(const Iterator& rhs) const { return m_entry_it != rhs.m_entry_it; }

			bool is_valid() const { return m_entry_it->active; }
			ObjectPoolHandle get_handle() const;
		private:
			ObjectPoolManager& m_manager;
			EntryVector::iterator m_entry_it;
		};

		Iterator begin() { return Iterator(*this, m_entries.begin()); }
		Iterator end() { return Iterator(*this, m_entries.end()); }
	};
}
#endif