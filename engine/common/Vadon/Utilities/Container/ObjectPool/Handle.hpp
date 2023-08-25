#ifndef VADON_UTILITIES_CONTAINER_OBJECTPOOL_HANDLE_HPP
#define VADON_UTILITIES_CONTAINER_OBJECTPOOL_HANDLE_HPP
namespace Vadon::Utilities
{
	struct ObjectPoolHandle
	{
		uint32_t index = 0;
		uint16_t generation = 0;
		uint16_t pool_generation = 0;

		bool is_valid() const { return (generation != 0) && (pool_generation != 0); }
		void invalidate() { index = 0; generation = 0; pool_generation = 0; }

		bool operator==(const ObjectPoolHandle& rhs) const { return ((index == rhs.index) && (generation == rhs.generation) && (pool_generation == rhs.pool_generation)); }
		bool operator!=(const ObjectPoolHandle& rhs) const { return !(*this == rhs); }

		bool operator<(const ObjectPoolHandle& rhs) const { return (index < rhs.index); } // Use the index

		uint64_t to_uint() const 
		{
			uint64_t value = index;
			value |= (static_cast<uint64_t>(generation) << 32);
			value |= (static_cast<uint64_t>(pool_generation) << 48);

			return value;
		}
	};

	template<typename T>
	struct TypedPoolHandle
	{
		ObjectPoolHandle handle;

		bool operator==(const TypedPoolHandle& rhs) const { return handle == rhs.handle; }
		bool operator!=(const TypedPoolHandle& rhs) const { return handle != rhs.handle; }

		bool operator<(const TypedPoolHandle& rhs) const { return (handle < rhs.handle); }

		bool is_valid() const { return handle.is_valid(); }
		void invalidate() { handle.is_valid(); }
	};
}

#define VADON_DECLARE_TYPED_POOL_HANDLE(_type, _name) using _name = Vadon::Utilities::TypedPoolHandle<class _type>
#endif