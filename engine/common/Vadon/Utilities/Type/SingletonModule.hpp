#ifndef VADON_UTILITIES_TYPE_SINGLETONMODULE_HPP
#define VADON_UTILITIES_TYPE_SINGLETONMODULE_HPP
#include <Vadon/Utilities/Type/TypeList.hpp>
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