#ifndef VADON_UTILITIES_TYPEINFO_TYPELIST_TYPELIST_HPP
#define VADON_UTILITIES_TYPEINFO_TYPELIST_TYPELIST_HPP
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
			static_assert(has_type<T>() == true, "Requested type not in type list!");
			return internal_get_type_id<T>();
		}

		template<typename T> static constexpr bool has_type()
		{
			return (std::is_same_v<T, Types> || ...);
		}
	private:
		template<typename T> static constexpr size_t internal_get_type_id()
		{
			static_assert(type_count() > 0, "Type list must not be empty!");

			// Make sure we have no duplicates in the list (NOTE: unfortunately this can only detect duplicates if they are actually being used,
			// but we expect this will be the case anyway)
			constexpr size_t types_matched = (... + (1 & std::is_same_v<T, Types>));
			static_assert(types_matched <= 1, "Duplicate entries in type list!");

			// Use fold expr to find the type (only evaluated once, and the benefits are worth the compile time cost)
			size_t counter = 0;
			bool found = (... || (++counter, std::is_same_v<T, Types>));
			return found ? counter - 1 : 0;
		}
	};

	template <typename T, typename... U>
	concept IsAnyOf = (std::same_as<T, U> || ...);

	template <typename T, typename... Alts>
	static constexpr size_t type_list_index_v = TypeList<Alts...>::template get_type_id<T>();

	template<typename T, typename... Alts>
	static constexpr bool type_list_has_type_v = Utilities::TypeList<Alts...>::template has_type<T>();
}
#endif