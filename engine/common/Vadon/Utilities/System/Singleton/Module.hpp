#ifndef VADON_UTILITIES_SYSTEM_SINGLETON_MODULE_HPP
#define VADON_UTILITIES_SYSTEM_SINGLETON_MODULE_HPP
#include <Vadon/Utilities/TypeInfo/TypeList.hpp>
namespace Vadon::Utilities
{
	template<typename ModuleList, typename ModuleImpl, typename... Types>
	class SingletonModule
	{
	public:
		using _Module = ModuleImpl;
		using _TypeList = TypeList<Types...>;

		static constexpr size_t get_module_id() { return ModuleList::template get_type_id<_Module>(); }

		template<typename T>
		static constexpr size_t get_type_id() { return _TypeList::template get_type_id<T>(); }
	};
}
#endif