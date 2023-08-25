#ifndef VADON_UTILITIES_CONTAINER_OBJECTPOOL_POOL_HPP
#define VADON_UTILITIES_CONTAINER_OBJECTPOOL_POOL_HPP
#include <Vadon/Utilities/Container/ObjectPool/Manager.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
namespace Vadon::Utilities
{
	template<typename HandleType, typename PoolType>
	class ObjectPool
	{
	public:
		using _Pool = ObjectPool<HandleType, PoolType>;
		using _Type = std::remove_pointer_t<PoolType>;
		using _PoolIterator = typename std::vector<PoolType>::iterator;
		using _Handle = TypedPoolHandle<HandleType>;

		using ObjectPair = std::pair<_Handle, _Type*>; // FIXME: use reference when object is not a pointer

		ObjectPool()
		{
			static_assert(!std::is_reference_v<PoolType>);
		}

		~ObjectPool()
		{
			reset();
		}

		void reset()
		{
			m_manager.reset();

			if constexpr (std::is_pointer_v<PoolType>)
			{
				for (_Type* current_object : m_pool)
				{
					if (current_object)
					{
						delete current_object;
					}
				}
			}

			m_pool.clear();
		}

		template<typename... Args>
		_Handle add(Args&&... args)
		{
			const ObjectPoolHandle new_handle = m_manager.add();

			if constexpr (std::is_pointer_v<PoolType>)
			{
				_Type* new_object = new _Type(std::forward<Args>(args)...);
				if (new_handle.index < static_cast<uint32_t>(m_pool.size()))
				{
					m_pool[new_handle.index] = new_object;
				}
				else
				{
					m_pool.push_back(new_object);
				}
			}
			else
			{
				if (new_handle.index < static_cast<uint32_t>(m_pool.size()))
				{
					m_pool[new_handle.index] = _Type(std::forward<Args>(args)...);
				}
				else
				{
					m_pool.emplace_back(std::forward<Args>(args)...);
				}
			}

			_Handle new_typed_handle;
			new_typed_handle.handle = new_handle;

			return new_typed_handle;
		}

		const _Type* get(const _Handle& handle) const
		{
			if (m_manager.validate(handle.handle))
			{
				return get_unchecked(handle);
			}

			return nullptr;
		}

		const _Type* get_unchecked(const _Handle& handle) const
		{
			if constexpr (std::is_pointer_v<PoolType>)
			{
				return m_pool[handle.handle.index];
			}
			else
			{
				return &(m_pool[handle.handle.index]);
			}
		}

		_Type* get(const _Handle& handle)
		{
			return const_cast<_Type*>(const_cast<const _Pool*>(this)->get(handle));
		}

		_Type* get_unchecked(const _Handle& handle)
		{
			return const_cast<_Type*>(const_cast<const _Pool*>(this)->get_unchecked(handle));
		}

		bool remove(const _Handle& handle)
		{
			if (m_manager.remove(handle.handle))
			{
				if constexpr (std::is_pointer_v<PoolType>)
				{
					delete m_pool[handle.handle.index];
					m_pool[handle.handle.index] = nullptr;
				}
				return true;
			}

			return false;
		}

		class Iterator
		{
		public:
			ObjectPair operator*() const
			{
				_Handle object_handle;
				object_handle.handle = m_manager_it.get_handle();

				if constexpr (std::is_pointer_v<PoolType>)
				{
					return ObjectPair(object_handle, *m_pool_it);
				}
				else
				{
					return ObjectPair(object_handle, m_manager_it.is_valid() ? &(*m_pool_it) : nullptr);
				}
			}

			Iterator& operator++()
			{
				// Advance until we get to a valid element
				++m_manager_it;
				++m_pool_it;

				const ObjectPoolManager::Iterator manager_end = m_manager.end();

				while ((m_manager_it != manager_end) && !m_manager_it.is_valid())
				{
					++m_manager_it;
					++m_pool_it;
				}

				return *this;
			}

			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

			bool operator==(const Iterator& rhs) const { return m_pool_it == rhs.m_pool_it; }
			bool operator!=(const Iterator& rhs) const { return m_pool_it != rhs.m_pool_it; }
		private:
			Iterator(ObjectPoolManager& manager, ObjectPoolManager::Iterator manager_it, _PoolIterator pool_it) : m_manager(manager), m_manager_it(manager_it), m_pool_it(pool_it) {}

			ObjectPoolManager& m_manager;
			ObjectPoolManager::Iterator m_manager_it;
			_PoolIterator m_pool_it;

			friend _Pool;
		};

		Iterator begin() { return Iterator(m_manager, m_manager.begin(), m_pool.begin()); }
		Iterator end() { return Iterator(m_manager, m_manager.end(), m_pool.end()); }
	private:
		ObjectPoolManager m_manager;
		std::vector<PoolType> m_pool;
	};
}
#endif