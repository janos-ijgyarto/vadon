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
		if constexpr (std::is_base_of_v<Vadon::Scene::ResourceID, T> && (std::is_same_v<Vadon::Scene::ResourceID, T> == false))
		{
			return ErasedDataTypeID{ .type = ErasedDataType::RESOURCE_ID,
				.id = Vadon::Utilities::to_integral(Vadon::Utilities::TypeRegistry::get_type_id<typename T::_ResourceType>()) };
		}
		else if constexpr (std::is_base_of_v<Vadon::Scene::ResourceHandle, T> && (std::is_same_v<Vadon::Scene::ResourceHandle, T> == false))
		{
			return ErasedDataTypeID{ .type = ErasedDataType::RESOURCE_HANDLE,
				.id = Vadon::Utilities::to_integral(Vadon::Utilities::TypeRegistry::get_type_id<typename T::_ResourceType>()) };
		}
		else if constexpr (type_list_has_type_v<variant_type_mapping_t<T>, Variant>)
		{
			return ErasedDataTypeID{ .type = ErasedDataType::TRIVIAL,
				.id = type_list_index_v<variant_type_mapping_t<T>, Variant> };
		}
		else
		{
			static_assert(false, "Type not supported!");
		}
	}
}
#endif