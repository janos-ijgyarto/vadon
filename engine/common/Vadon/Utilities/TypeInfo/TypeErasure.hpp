#ifndef VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#define VADON_UTILITIES_TYPEINFO_TYPEERASURE_HPP
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	static constexpr ErasedDataTypeID get_erased_data_type_id()
	{
		if constexpr (Vadon::Scene::is_resource_id<T>)
		{
			return ErasedDataTypeID{ .type = ErasedDataType::RESOURCE_ID,
				.id = Vadon::Utilities::to_integral(Vadon::Utilities::TypeRegistry::get_type_id<typename T::_ResourceType>()) };
		}
		else if constexpr (is_std_vector<T>)
		{
			static_assert(is_std_vector<typename T::value_type> == false, "Nested arrays are not supported!");
			return ErasedDataTypeID{ .type = ErasedDataType::ARRAY, .id = 0 };
		}
		else if constexpr (is_trivial_variant_type<T>)
		{
			return ErasedDataTypeID{ .type = ErasedDataType::TRIVIAL, .id = type_list_index_v<T, Variant> };
		}
		else
		{
			static_assert(false, "Type not supported!");
		}
	}

	VADONCOMMON_API Variant get_erased_type_default_value(const ErasedDataTypeID& type_id);
}
#endif