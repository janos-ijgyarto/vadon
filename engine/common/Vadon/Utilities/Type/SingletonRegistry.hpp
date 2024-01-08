#ifndef VADON_UTILITIES_TYPE_SINGLETONREGISTRY_HPP
#define VADON_UTILITIES_TYPE_SINGLETONREGISTRY_HPP
#include <Vadon/Common.hpp>
#include <vector>
namespace Vadon::Utilities
{
	class SingletonBase;

	// Compile-time registry that can map to one instance per-type. Instances are expected to be registered at load time
	class SingletonRegistry
	{
	public:
		template<typename T> const T& get() const
		{
			return *static_cast<const T*>(get_internal(T::_Module::get_module_id(), T::get_type_id()));
		}

		template<typename T> T& get()
		{
			return const_cast<T&>(const_cast<const SingletonRegistry*>(this)->get<T>());
		}

	protected:
		VADONCOMMON_API void register_instance(SingletonBase* instance, size_t module_index, size_t type_index);
	private:
		VADONCOMMON_API const SingletonBase* get_internal(size_t module_index, size_t type_index) const;

		using OffsetList = std::vector<size_t>;
		std::vector<OffsetList> m_module_instance_offsets;
		std::vector<SingletonBase*> m_instances;

		friend SingletonBase;
	};
}
#endif