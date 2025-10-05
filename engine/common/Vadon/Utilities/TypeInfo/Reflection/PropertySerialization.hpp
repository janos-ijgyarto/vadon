#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>
namespace Vadon::Utilities
{
	template<typename T>
	constexpr size_t variant_type_list_index_v = Vadon::Utilities::type_list_index_v<T, Vadon::Utilities::Variant>;

	// TODO: default value?
	template<typename T, typename Key>
	bool serialize_trivial_property(Vadon::Utilities::Serializer& serializer, Key key, Vadon::Utilities::Variant& property_value, Vadon::Utilities::Serializer::Result& result, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		if (data_type.id != variant_type_list_index_v<T>)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			// TODO: allow setting default value if not found?
			T value;
			result = serializer.serialize(key, value);
			if (result == Vadon::Utilities::Serializer::Result::SUCCESSFUL)
			{
				property_value = value;
			}
		}
		else
		{
			T& value = std::get<T>(property_value);
			result = serializer.serialize(key, value);
		}

		return true;
	}

	template <typename... Types>
	Vadon::Utilities::Serializer::Result serialize_trivial_property_fold(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		Vadon::Utilities::Serializer::Result result;
		const bool fold_result = (serialize_trivial_property<Types, std::string_view>(serializer, property_name, property_value, result, data_type) || ...);
		if (fold_result == false)
		{
			result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
		}

		return result;
	}

	template <typename... Types>
	Vadon::Utilities::Serializer::Result serialize_trivial_property_fold(Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		Vadon::Utilities::Serializer::Result result;
		const bool fold_result = (serialize_trivial_property<Types, size_t>(serializer, index, property_value, result, data_type) || ...);
		if (fold_result == false)
		{
			result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
		}

		return result;
	}

	inline Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view key, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		return serialize_trivial_property_fold<int, float, bool, std::string, Vadon::Math::Vector2, Vadon::Math::Vector2i, Vadon::Math::Vector3, Vadon::Math::ColorRGBA, Vadon::Utilities::UUID>(serializer, key, property_value, data_type);
	}

	inline Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
	{
		return serialize_trivial_property_fold<int, float, bool, std::string, Vadon::Math::Vector2, Vadon::Math::Vector2i, Vadon::Math::Vector3, Vadon::Math::ColorRGBA, Vadon::Utilities::UUID>(serializer, index, property_value, data_type);
	}
}
#endif