#ifndef VADON_UTILITIES_SYSTEM_SINGLETON_SINGLETON_HPP
#define VADON_UTILITIES_SYSTEM_SINGLETON_SINGLETON_HPP
#include <Vadon/Utilities/System/Singleton/Registry.hpp>
namespace Vadon::Utilities
{
	class SingletonBase
	{
	protected:
		SingletonBase(SingletonRegistry& registry, size_t module_index, size_t type_index)
		{
			registry.register_instance(this, module_index, type_index);
		}
	};

	template<typename Module, typename Impl>
	class Singleton : public SingletonBase
	{
	public:
		using _Module = Module;

		static constexpr size_t get_type_id() { return _Module::template get_type_id<Impl>(); }
	protected:
		Singleton(SingletonRegistry& registry)
			: SingletonBase(registry, _Module::get_module_id(), _Module::template get_type_id<Impl>())
		{
		}
	};
}
#endif