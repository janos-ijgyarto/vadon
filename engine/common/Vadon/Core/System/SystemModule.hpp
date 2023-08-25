#ifndef VADON_CORE_SYSTEM_SYSTEMMODULE_HPP
#define VADON_CORE_SYSTEM_SYSTEMMODULE_HPP
#include <Vadon/Utilities/Type/TypeList.hpp>
namespace Vadon::Core
{
	template<typename ModuleList, typename ModuleImpl, typename... Systems>
	class SystemModule
	{
	public:
		using _Module = ModuleImpl;
		using SystemList = Vadon::Utilities::TypeList<Systems...>;

		static constexpr size_t get_type_id() { return ModuleList::template get_type_id<_Module>(); }

		template<typename Sys>
		static constexpr size_t get_system_id() { return SystemList::template get_type_id<Sys>(); }
	};
}
#endif