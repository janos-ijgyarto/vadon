#ifndef VADON_UTILITIES_CONTAINER_OBJECTPOOL_POOL_HPP
#define VADON_UTILITIES_CONTAINER_OBJECTPOOL_POOL_HPP
#include <Vadon/Utilities/Container/ObjectPool/Manager.hpp>
#include <Vadon/Utilities/Debugging/Assert.hpp>
namespace Vadon::Utilities
{
	// TODO: implement "deque" (or "hive") version where memory address is always stable
	// Also move as much code as possible to non-templated code
	template<typename HandleType, typename PoolType>
	class ObjectPool
	{
	public:
		using _Pool = ObjectPool<HandleType, PoolType>;
		using _Type = std::remove_pointer_t<PoolType>;
		using _PoolConstIterator = typename std::vector<PoolType>::const_iterator;
		using _PoolIterator = typename std::vector<PoolType>::iterator;
		using _Handle = TypedPoolHandle<HandleType>;

		// FIXME: use reference when object is not a pointer
		using ConstObjectPair = std::pair<_Handle, const _Type*>;
		using ObjectPair = std::pair<_Handle, _Type*>;

		ObjectPool()
		{
			static_assert(std::is_reference_v<PoolType> == false, "Cannot use reference as pool data type!");
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

		const bool is_handle_valid(const _Handle& handle) const
		{
			return m_manager.validate(handle.handle);
		}

		const _Type& get(const _Handle& handle) const
		{
			// FIXME: could move the assert to manager by having it return the data index (in case we use "deque" implementation)
			VADON_ASSERT(is_handle_valid(handle) == true, "Tried to retrieve invalid handle!");
			if constexpr (std::is_pointer_v<PoolType>)
			{
				return *m_pool[handle.handle.index];
			}
			else
			{
				return m_pool[handle.handle.index];
			}
		}

		const _Type* get_safe(const _Handle& handle) const
		{
			if (is_handle_valid(handle) == true)
			{
				return get(handle);
			}

			return nullptr;
		}

		_Type& get(const _Handle& handle)
		{
			return const_cast<_Type&>(std::as_const(*this).get(handle));
		}

		_Type* get_safe(const _Handle& handle)
		{
			return const_cast<_Type*>(std::as_const(*this).get_safe(handle));
		}

		void remove(const _Handle& handle)
		{
			m_manager.remove(handle.handle);

			// TODO: run destructor on object even if it's not a pointer?
			if constexpr (std::is_pointer_v<PoolType>)
			{
				delete m_pool[handle.handle.index];
				m_pool[handle.handle.index] = nullptr;
			}
		}

		// FIXME: replace with ECS query iterator-style API
		class ConstIterator
		{
		public:
			ConstObjectPair operator*() const
			{
				_Handle object_handle;
				object_handle.handle = m_manager_it.get_handle();

				if constexpr (std::is_pointer_v<PoolType>)
				{
					return ConstObjectPair(object_handle, *m_pool_it);
				}
				else
				{
					return ConstObjectPair(object_handle, m_manager_it.is_valid() ? &(*m_pool_it) : nullptr);
				}
			}

			ConstIterator& operator++()
			{
				// Advance until we get to a valid element
				++m_manager_it;
				++m_pool_it;

				advance();

				return *this;
			}

			ConstIterator operator++(int) { ConstIterator tmp = *this; ++(*this); return tmp; }

			bool operator==(const ConstIterator& rhs) const { return m_pool_it == rhs.m_pool_it; }
			bool operator!=(const ConstIterator& rhs) const { return m_pool_it != rhs.m_pool_it; }
		protected:
			ConstIterator(const ObjectPoolManager& manager, ObjectPoolManager::Iterator manager_it, _PoolConstIterator pool_it) : m_manager(manager), m_manager_it(manager_it), m_pool_it(pool_it) { advance(); }

			void advance()
			{
				const ObjectPoolManager::Iterator manager_end = m_manager.end();
				while (m_manager_it != manager_end)
				{
					if (m_manager_it.is_valid() == true)
					{
						break;
					}
					++m_manager_it;
					++m_pool_it;
				}
			}

			const ObjectPoolManager& m_manager;
			ObjectPoolManager::Iterator m_manager_it;
			_PoolConstIterator m_pool_it;

			friend _Pool;
		};

		// FIXME: replace with ECS query iterator-style API
		class Iterator : public ConstIterator
		{
		public:
			using ConstIterator::ConstIterator;

			ObjectPair operator*() const
			{
				ConstObjectPair const_pair = ConstIterator::operator*();
				return ObjectPair(const_pair.first, const_cast<_Type*>(const_pair.second));
			}

			Iterator& operator++()
			{
				ConstIterator::operator++();
				return *this;
			}

			Iterator operator++(int)
			{
				Iterator tmp = *this;
				ConstIterator::operator++();
				return tmp;
			}

			bool operator==(const Iterator& rhs) const { return ConstIterator::operator==(rhs); }
			bool operator!=(const Iterator& rhs) const { return ConstIterator::operator!=(rhs); }
		};

		ConstIterator begin() const { return ConstIterator(m_manager, m_manager.begin(), m_pool.cbegin()); }
		Iterator begin() { return Iterator(m_manager, m_manager.begin(), m_pool.cbegin()); }

		ConstIterator end() const { return ConstIterator(m_manager, m_manager.end(), m_pool.cend()); }
		Iterator end() { return Iterator(m_manager, m_manager.end(), m_pool.cend()); }
	private:
		ObjectPoolManager m_manager;
		std::vector<PoolType> m_pool;
	};
}
#endif