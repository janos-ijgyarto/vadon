#ifndef VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	struct TypeErasureTrait
	{
		static constexpr TypeID get_erased_type_id()
		{
			static_assert(false, "Type not supported!");
			return TypeID::INVALID;
		}

		static constexpr TypeID get_underlying_type_id()
		{
			static_assert(false, "Type not supported!");
			return TypeID::INVALID;
		}
	};

	template<is_trivial_variant_type T>
	struct TypeErasureTrait<T>
	{
		static constexpr TypeID get_erased_type_id()
		{
			return TypeRegistry::get_type_id<T>();
		}

		static constexpr TypeID get_underlying_type_id()
		{
			return TypeRegistry::get_type_id<T>();
		}
	};

	template<is_std_vector T>
	struct TypeErasureTrait<T>
	{
		static constexpr TypeID get_erased_type_id()
		{
			static_assert(is_std_vector<typename T::value_type> == false, "Nested arrays are not supported!");
			return TypeRegistry::get_type_id<BoxedVariantArray>();
		}

		static constexpr TypeID get_underlying_type_id()
		{
			static_assert(is_std_vector<typename T::value_type> == false, "Nested arrays are not supported!");
			return TypeErasureTrait<typename T::value_type>::get_underlying_type_id();
		}
	};

	template<typename T>
	struct ObjectTypeErasureTrait
	{
		static constexpr TypeID get_erased_type_id()
		{
			return TypeRegistry::get_type_id<ObjectPointer>();
		}

		static constexpr TypeID get_underlying_type_id()
		{
			return TypeRegistry::get_type_id<T>();
		}
	};

	VADONCOMMON_API TypeID get_erased_data_type_id(TypeID type_id);
	VADONCOMMON_API Variant get_erased_type_default_value(TypeID type_id);
}

#define VADON_DEFINE_OBJECT_TYPE_ERASURE(_type) template<>\
struct Vadon::Utilities::TypeErasureTrait<_type>\
{\
	static constexpr TypeID get_erased_type_id()\
	{\
		return ObjectTypeErasureTrait<_type>::get_erased_type_id();\
	}\
	static constexpr TypeID get_underlying_type_id()\
	{\
		return ObjectTypeErasureTrait<_type>::get_underlying_type_id();\
	}\
}
#endif