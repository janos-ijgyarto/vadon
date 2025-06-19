#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	constexpr size_t variant_type_list_index_v = Vadon::Utilities::type_list_index_v<T, Vadon::Utilities::Variant>;

	// TODO: default value?
	template<typename T>
	bool serialize_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::Serializer::Result& result, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		if (data_type.id != variant_type_list_index_v<T>)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			// TODO: allow setting default value if not found?
			T value;
			result = serializer.serialize(property_name, value);
			if (result == Vadon::Utilities::Serializer::Result::SUCCESSFUL)
			{
				property_value = value;
			}
		}
		else
		{
			T& value = std::get<T>(property_value);
			result = serializer.serialize(property_name, value);
		}

		return true;
	}

	template <typename... Types>
	Vadon::Utilities::Serializer::Result serialize_trivial_property_fold(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		Vadon::Utilities::Serializer::Result result;
		const bool fold_result = (serialize_trivial_property<Types>(serializer, property_name, property_value, result, data_type) || ...);
		if (fold_result == false)
		{
			result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
		}

		return result;
	}

	inline Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		return serialize_trivial_property_fold<int, float, bool, std::string, Vadon::Utilities::Vector2, Vadon::Utilities::Vector2i, Vadon::Utilities::Vector3, Vadon::Utilities::ColorRGBA, Vadon::Utilities::UUID>(serializer, property_name, property_value, data_type);
	}
}
#endif