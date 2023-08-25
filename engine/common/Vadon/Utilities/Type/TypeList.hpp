#ifndef VADON_UTILITIES_TYPE_TYPELIST_HPP
#define VADON_UTILITIES_TYPE_TYPELIST_HPP
#include <tuple>
namespace Vadon::Utilities
{
	// Type list for generating sequential unique type IDs at compile time. Users can declare a type list using forward declared types.
	// The object uses fold expressions to generate sequential IDs for all types registered in the variadic template arguments.
	// Can detect redundancies and other issues at compile time.
	template <typename... Types>
	struct TypeList
	{
	public:
		static constexpr size_t type_count() { return sizeof...(Types); }

		template<typename T> static constexpr size_t get_type_id()
		{
			static_assert(type_count() > 0, "Type list must not be empty!");

			// Make sure we have no duplicates in the list (NOTE: unfortunately this can only detect duplicates if they are actually being used,
			// but we expect this will be the case anyway)
			constexpr size_t types_matched = (... + (1 & std::is_same_v<T, Types>));
			static_assert(types_matched <= 1);
			static_assert((std::is_same_v<T, Types> || ...), "Requested type not in type list!");

			// Use fold expr to find the type (only evaluated once, and the benefits are worth the compile time cost)
			size_t counter = 0;
			bool found = (... || (++counter, std::is_same_v<T, Types>));
			return found ? counter - 1 : 0;
		}
	};
}
#endif