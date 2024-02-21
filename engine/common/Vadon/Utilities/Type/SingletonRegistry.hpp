#ifndef VADON_UTILITIES_TYPE_SINGLETONREGISTRY_HPP
#define VADON_UTILITIES_TYPE_SINGLETONREGISTRY_HPP
#include <Vadon/Common.hpp>
#include <vector>
namespace Vadon::Utilities
{
	class SingletonBase;

	// Compile-time registry that can map to one instance per-type. Instances are expected to be registered at load time
	// FIXME: could revise the storage to better pack the system pointers and have fewer indirections!
	// (Initially load them into an array, storing metadata, then sort and cache the offsets)
	class SingletonRegistry
	{
	public:
		template<typename T> const T& get() const
		{
			return *static_cast<const T*>(get_base(get_offset<T>()));
		}

		template<typename T> T& get()
		{
			return const_cast<T&>(const_cast<const SingletonRegistry*>(this)->get<T>());
		}

		template<typename T> size_t get_offset() const
		{
			return get_offset_internal(T::_Module::get_module_id(), T::get_type_id());
		}

		SingletonBase* get_base(size_t offset) const { return m_instances[offset]; }
	protected:
		VADONCOMMON_API void register_instance(SingletonBase* instance, size_t module_index, size_t type_index);
	private:
		VADONCOMMON_API size_t get_offset_internal(size_t module_index, size_t type_index) const;

		using OffsetList = std::vector<size_t>;
		std::vector<OffsetList> m_module_instance_offsets;
		std::vector<SingletonBase*> m_instances;

		friend SingletonBase;
	};
}
#endif