#ifndef VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#define VADON_UTILITIES_TYPEINFO_REFLECTION_PROPERTYSERIALIZATION_HPP
#include <Vadon/Foundation/TypeInfo/TypeInfo.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>
namespace Vadon::Utilities
{
	// TODO: default value?
	template<typename T, typename Key>
	Vadon::Utilities::Serializer::Result serialize_trivial_property(Vadon::Utilities::Serializer& serializer, Key key, Vadon::Utilities::Variant& property_value)
	{
		Vadon::Utilities::Serializer::Result result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
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

		return result;
	}

	// FIXME: find a more elegant solution where we can "register" each of these trivial types
	// and search for the relevant function based on data type
	template<typename Key>
	inline Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, Key key, Vadon::Utilities::Variant& property_value, Vadon::Utilities::TypeID data_type)
	{
		const Vadon::Foundation::BaseType base_type = static_cast<Vadon::Foundation::BaseType>(data_type);
		switch (base_type)
		{
		case Vadon::Foundation::BaseType::INT32:
			return serialize_trivial_property<int, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::UINT32:
			return serialize_trivial_property<uint32_t, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::FLOAT:
			return serialize_trivial_property<float, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::BOOL:
			return serialize_trivial_property<bool, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::STRING:
			return serialize_trivial_property<std::string, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::VECTOR2:
			return serialize_trivial_property<Vadon::Math::Vector2, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::VECTOR2I:
			return serialize_trivial_property<Vadon::Math::Vector2i, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::VECTOR3:
			return serialize_trivial_property<Vadon::Math::Vector3, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::VECTOR3I:
			return serialize_trivial_property<Vadon::Math::Vector3i, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::VECTOR4:
			return serialize_trivial_property<Vadon::Math::Vector4, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::COLORRGBA:
			return serialize_trivial_property<Vadon::Math::ColorRGBA, Key>(serializer, key, property_value);
		case Vadon::Foundation::BaseType::UUID:
			return serialize_trivial_property<Vadon::Foundation::UUID, Key>(serializer, key, property_value);
		default:
			VADON_ERROR("Invalid property type!");
			break;
		}

		return Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
	}
}
#endif