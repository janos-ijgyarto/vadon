#ifndef VADON_UTILITIES_TYPEINFO_TYPELIST_VARIANTTYPELIST_HPP
#define VADON_UTILITIES_TYPEINFO_TYPELIST_VARIANTTYPELIST_HPP
#include <Vadon/Utilities/TypeInfo/TypeList/TypeList.hpp>
#include <variant>
namespace Vadon::Utilities
{
	template <typename T, typename... Alts>
	static constexpr size_t type_list_index_v<T, std::variant<Alts...>> = TypeList<Alts...>::template get_type_id<T>();

	template <typename T, typename... Alts>
	static constexpr bool type_list_has_type_v<T, std::variant<Alts...>> = Utilities::TypeList<Alts...>::template has_type<T>();
}
#endif