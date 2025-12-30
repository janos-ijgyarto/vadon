#ifndef VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	// Utility function to ensure the type is Variant-compatible
	template<typename T>
	static constexpr TypeID get_erased_data_type_id()
	{
		if constexpr (is_trivial_variant_type<T>)
		{
			return TypeRegistry::get_type_id<T>();
		}
		else if constexpr (is_std_vector<T>)
		{
			static_assert(is_std_vector<typename T::value_type> == false, "Nested arrays are not supported!");
			return TypeRegistry::get_type_id<BoxedVariantArray>();
		}
		else
		{
			static_assert(false, "Type not supported!");
		}
	}

	VADONCOMMON_API TypeID get_erased_data_type_id(TypeID type_id);
	VADONCOMMON_API Variant get_erased_type_default_value(TypeID type_id);
}
#endif