#include <Vadon/Utilities/Data/ObjectSerializer.hpp>

#include <Vadon/Utilities/Data/Object.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

namespace
{
	template<typename Key, typename T>
	bool serialize_base_type_impl(Vadon::Utilities::Serializer& serializer, Key key, Vadon::Utilities::Variant& value)
	{
		T temp_value;

		if (serializer.is_reading() == false)
		{
			temp_value = std::get<T>(value);
		}

		if (serializer.serialize(key, temp_value) != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			value = temp_value;
		}

		return true;
	}

	template<typename Key>
	bool serialize_base_type(Vadon::Utilities::Serializer& serializer, ::Vadon::Foundation::BaseType base_type, Key key, Vadon::Utilities::Variant& value)
	{
		using BaseType = ::Vadon::Foundation::BaseType;
		switch (base_type)
		{
		case BaseType::INT32:
			return serialize_base_type_impl<Key, int>(serializer, key, value);
		case BaseType::UINT32:
			return serialize_base_type_impl<Key, uint32_t>(serializer, key, value);
		case BaseType::FLOAT:
			return serialize_base_type_impl<Key, float>(serializer, key, value);
		case BaseType::BOOL:
			return serialize_base_type_impl<Key, bool>(serializer, key, value);
		case BaseType::STRING:
			return serialize_base_type_impl<Key, std::string>(serializer, key, value);
		case BaseType::VECTOR2:
			return serialize_base_type_impl<Key, Vadon::Math::Vector2>(serializer, key, value);
		case BaseType::VECTOR2I:
			return serialize_base_type_impl<Key, Vadon::Math::Vector2i>(serializer, key, value);
		case BaseType::VECTOR3:
			return serialize_base_type_impl<Key, Vadon::Math::Vector3>(serializer, key, value);
		case BaseType::VECTOR3I:
			return serialize_base_type_impl<Key, Vadon::Math::Vector3i>(serializer, key, value);
		case BaseType::VECTOR4:
			return serialize_base_type_impl<Key, Vadon::Math::Vector4>(serializer, key, value);
		case BaseType::COLORRGBA:
			return serialize_base_type_impl<Key, Vadon::Math::ColorRGBA>(serializer, key, value);
		case BaseType::UUID:
			return serialize_base_type_impl<Key, ::Vadon::Foundation::UUID>(serializer, key, value);
		default:
			return false;
		}
	}

	bool serialize_object_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view key, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value)
	{
		const ::Vadon::Foundation::BaseType base_type = Vadon::Utilities::base_type_from_uuid(property_info.base_info.root_type);
		return serialize_base_type<std::string_view>(serializer, base_type, key, property_value);
	}

	bool serialize_object_array_nested_array_element(Vadon::Utilities::Serializer& /*serializer*/, size_t /*index*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, Vadon::Utilities::Variant& /*array_element_value*/)
	{
		// FIXME: not supported yet (but it's possible now!)
		return false;
	}

	bool serialize_object_array_nested_dictionary_element(Vadon::Utilities::Serializer& /*serializer*/, size_t /*index*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, Vadon::Utilities::Variant& /*array_element_value*/)
	{
		// FIXME: not supported yet (but it's possible now!)
		return false;
	}

	template<typename Key>
	bool serialize_object_subobject_impl(Vadon::Utilities::Serializer& serializer, Key key, Vadon::Utilities::Variant& value, const ::Vadon::Foundation::UUID& object_type, bool allow_subclass)
	{
		Vadon::Utilities::VariantDictionary object_dictionary;
		if (serializer.is_reading() == false)
		{
			object_dictionary = *std::get<Vadon::Utilities::BoxedVariantDictionary>(value);
		}

		if (serializer.open_object(key) != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
		{
			return false;
		}

		// Check if it's a generic or explicit object
		if (((object_type.is_valid() == true) && (allow_subclass == true)) || (object_type.is_valid() == false))
		{
			// FIXME: also validate when serializing?
			if ((object_type.is_valid() == true) && (serializer.is_reading() == true))
			{
				// Ensure that the deserialized type is compatible
				constexpr Vadon::Foundation::UUID type_property_uuid = Vadon::Utilities::Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_type_property);
				::Vadon::Utilities::TypeUUID type_uuid;
				if (serializer.serialize(type_property_uuid, type_uuid) != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
				{
					return false;
				}

				if (Vadon::Utilities::TypeRegistry::is_base_of(object_type, type_uuid) == false)
				{
					return false;
				}
			}

			if (Vadon::Utilities::ObjectSerializer::serialize_object(serializer, object_dictionary) == false)
			{
				return false;
			}
		}
		else if (object_type.is_valid() == true)
		{
			// Assume we are processing the object properties
			const Vadon::Utilities::TypeID object_type_id = Vadon::Utilities::TypeRegistry::get_type_id(object_type);
			if (Vadon::Utilities::ObjectSerializer::serialize_object_properties(serializer, object_type_id, object_dictionary) == false)
			{
				return false;
			}
		}
		else
		{
			VADON_ERROR("Invalid parameters!");
		}

		if (serializer.close_object() != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			value = Vadon::Utilities::Box(object_dictionary);
		}

		return true;
	}

	bool serialize_object_array_subobject_element(Vadon::Utilities::Serializer& serializer, size_t index, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& array_element_value)
	{
		// Check if the array uses ObjectWrapper
		const ::Vadon::Utilities::TypeUUID element_type = property_info.type_list[type_list_offset];
		if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			const size_t object_type_offset = type_list_offset + 1;
			if (object_type_offset >= property_info.type_list.size())
			{
				// No type specified, so it's a generic object
				return serialize_object_subobject_impl<size_t>(serializer, index, array_element_value, Vadon::Utilities::TypeUUID{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const ::Vadon::Utilities::TypeUUID object_type = property_info.type_list[object_type_offset];
				return serialize_object_subobject_impl<size_t>(serializer, index, array_element_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return serialize_object_subobject_impl<size_t>(serializer, index, array_element_value, Vadon::Utilities::TypeUUID{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return serialize_object_subobject_impl<size_t>(serializer, index, array_element_value, element_type, false);
		}
	}

	bool serialize_object_array_resource_element(Vadon::Utilities::Serializer& serializer, size_t index, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& array_element_value)
	{
		const size_t resource_type_offset = type_list_offset + 1;
		VADON_ASSERT(resource_type_offset < property_info.type_list.size(), "Missing resource type!");

		// Serialize the array element itself
		if (serialize_base_type<size_t>(serializer, ::Vadon::Foundation::BaseType::UUID, index, array_element_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?
		//const Vadon::Utilities::TypeUUID resource_type_uuid = property_info.type_list[resource_type_offset];

		return true;
	}

	bool serialize_object_array_property_element(Vadon::Utilities::Serializer& serializer, size_t index, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& array_element_value)
	{
		const ::Vadon::Foundation::UUID element_type = property_info.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = Vadon::Utilities::PropertyInfo::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			const ::Vadon::Foundation::BaseType base_type = Vadon::Utilities::base_type_from_uuid(element_type);
			return serialize_base_type<size_t>(serializer, base_type, index, array_element_value);
		}
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return serialize_object_array_nested_array_element(serializer, index, property_info, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return serialize_object_array_nested_dictionary_element(serializer, index, property_info, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return serialize_object_array_subobject_element(serializer, index, property_info, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return serialize_object_array_resource_element(serializer, index, property_info, type_list_offset, array_element_value);
		default:
			// Unsupported category!
			return false;
		}
	}

	bool serialize_object_array_property(Vadon::Utilities::Serializer& serializer, std::string_view key, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value)
	{
		VADON_ASSERT(property_info.type_list.front() == Vadon::Utilities::get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY), "Invalid type!");

		if (serializer.open_array(key) != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
		{
			return false;
		}

		Vadon::Utilities::VariantArray array_data;
		if (serializer.is_reading() == false)
		{
			array_data = *std::get<Vadon::Utilities::BoxedVariantArray>(property_value);
		}

		const size_t array_size = serializer.is_reading() ? serializer.get_array_size() : array_data.data.size();
		for (size_t index = 0; index < array_size; ++index)
		{
			Vadon::Utilities::Variant array_element_value;
			if (serializer.is_reading() == false)
			{
				array_element_value = array_data.data[index];
			}

			if (serialize_object_array_property_element(serializer, index, property_info, 1, array_element_value) == false)
			{
				return false;
			}

			if (serializer.is_reading() == true)
			{
				array_data.data.push_back(array_element_value);
			}
		}

		if (serializer.close_array() != Vadon::Utilities::Serializer::Result::SUCCESSFUL)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			property_value = Vadon::Utilities::Box(array_data);
		}

		return true;
	}

	bool serialize_object_dictionary_property(Vadon::Utilities::Serializer& /*serializer*/, std::string_view /*key*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, Vadon::Utilities::Variant& /*property_value*/)
	{
		// FIXME: implement support for dictionary!
		return false;
	}

	bool serialize_object_subobject_property(Vadon::Utilities::Serializer& serializer, std::string_view key, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value)
	{
		// Check if the property uses ObjectWrapper
		const Vadon::Utilities::TypeUUID element_type = property_info.type_list.front();
		if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_info.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return serialize_object_subobject_impl<std::string_view>(serializer, key, property_value, Vadon::Utilities::TypeUUID{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const Vadon::Utilities::TypeUUID object_type = property_info.type_list[1];
				return serialize_object_subobject_impl<std::string_view>(serializer, key, property_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return serialize_object_subobject_impl<std::string_view>(serializer, key, property_value, Vadon::Utilities::TypeUUID{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return serialize_object_subobject_impl<std::string_view>(serializer, key, property_value, element_type, false);
		}

		return true;
	}

	bool serialize_object_resource_property(Vadon::Utilities::Serializer& serializer, std::string_view key, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value)
	{
		VADON_ASSERT(property_info.type_list.size() >= 2, "Missing resource type!");

		// Serialize the UUID property itself
		if (serialize_base_type<std::string_view>(serializer, ::Vadon::Foundation::BaseType::UUID, key, property_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?
		//const Vadon::Utilities::TypeUUID resource_type_uuid = property_info.type_list[resource_type_offset];

		return true;
	}

	bool serialize_object_property_value(Vadon::Utilities::Serializer& serializer, std::string_view key, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value)
	{
		const ::Vadon::Foundation::Property::Category property_category = Vadon::Utilities::PropertyInfo::get_category(property_info.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
			return serialize_object_trivial_property(serializer, key, property_info, property_value);
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return serialize_object_array_property(serializer, key, property_info, property_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return serialize_object_dictionary_property(serializer, key, property_info, property_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return serialize_object_subobject_property(serializer, key, property_info, property_value);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return serialize_object_resource_property(serializer, key, property_info, property_value);
		default:
			// Unsupported category!
			return false;
		}
	}
}

namespace Vadon::Utilities
{
	bool ObjectSerializer::serialize_object(Serializer& serializer, VariantDictionary& object_dictionary)
	{
		constexpr Vadon::Foundation::UUID type_property_uuid = Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_type_property);
		constexpr Vadon::Foundation::UUID properties_property_uuid = Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property);

		if (serializer.is_reading() == true)
		{
			// First check if it's a null object
			// FIXME: modify serializer API to allow checking without querying keys!
			const Serializer::KeyVector keys = serializer.get_keys();
			if (keys.empty() == true)
			{
				// Null object, we can clear the dictionary and early out
				object_dictionary.data.clear();
				return true;
			}

			::Vadon::Foundation::UUID object_type_uuid;
			if (serializer.serialize(type_property_uuid, object_type_uuid) != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}

			if (object_type_uuid.is_valid() == false)
			{
				return false;
			}

			const TypeID object_type_id = TypeRegistry::get_type_id(object_type_uuid);
			if (object_type_id == TypeID::INVALID)
			{
				return false;
			}

			object_dictionary.data.insert(std::make_pair(uuid_to_base64_string(type_property_uuid), object_type_uuid));

			if (serializer.open_object(properties_property_uuid) != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}

			VariantDictionary properties_dictionary;
			if (serialize_object_properties(serializer, object_type_id, properties_dictionary) == false)
			{
				return false;
			}

			object_dictionary.data.insert(std::make_pair(uuid_to_base64_string(properties_property_uuid), Box(properties_dictionary)));

			if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}
		}
		else
		{
			if (object_dictionary.data.empty() == true)
			{
				// Empty dictionary means null object, early out
				return true;
			}

			auto obj_type_it = object_dictionary.data.find(uuid_to_base64_string(type_property_uuid));
			if (obj_type_it == object_dictionary.data.end())
			{
				return false;
			}

			::Vadon::Foundation::UUID object_type_uuid = std::get<::Vadon::Foundation::UUID>(obj_type_it->second);
			if (object_type_uuid.is_valid() == false)
			{
				return false;
			}

			const TypeID object_type_id = TypeRegistry::get_type_id(object_type_uuid);
			if (object_type_id == TypeID::INVALID)
			{
				return false;
			}

			auto obj_properties_it = object_dictionary.data.find(uuid_to_base64_string(type_property_uuid));
			if (obj_properties_it == object_dictionary.data.end())
			{
				return false;
			}

			BoxedVariantDictionary& properties_dictionary = std::get<BoxedVariantDictionary>(obj_properties_it->second);

			// FIXME: should we serialize with labels?
			if (serializer.serialize(type_property_uuid, object_type_uuid) != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}

			if (serializer.open_object(properties_property_uuid) != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}

			if (serialize_object_properties(serializer, object_type_id, *properties_dictionary) == false)
			{
				return false;
			}

			if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}
		}

		return true;
	}

	bool ObjectSerializer::serialize_object_properties(Serializer& serializer, TypeID object_type, VariantDictionary& object_properties)
	{
		if (serializer.is_reading() == true)
		{
			const Serializer::KeyVector keys = serializer.get_keys();
			for (const std::string& current_key : keys)
			{
				Vadon::Foundation::UUID current_property_id;
				if (Utilities::uuid_from_base64_string(current_key, current_property_id) == false)
				{
					return false;
				}

				const PropertyInfo property_info = TypeRegistry::get_property_info(object_type, current_property_id);
				if (property_info.base_info.is_valid() == false)
				{
					// TODO: add warning?
					continue;
				}

				Variant property_value;
				if (serialize_object_property_value(serializer, current_key, property_info, property_value) == false)
				{
					return false;
				}

				const std::string property_uuid_key = uuid_to_base64_string(current_property_id);
				object_properties.data.insert(std::make_pair(property_uuid_key, property_value));
			}
		}
		else
		{
			const PropertyInfoList property_info_list = TypeRegistry::get_type_properties(object_type);
			for (const PropertyInfo& current_property_info : property_info_list)
			{
				// FIXME: use labeled UUID?
				const std::string current_key = uuid_to_base64_string(current_property_info.base_info.id);

				auto property_data_it = object_properties.data.find(current_key);
				if (property_data_it == object_properties.data.end())
				{
					continue;
				}

				Variant& property_value = property_data_it->second;
				if (serialize_object_property_value(serializer, current_key, current_property_info, property_value) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool ObjectSerializer::load_object_data(ObjectWrapper& object, const VariantDictionary& data)
	{
		if (data.data.empty() == true)
		{
			// Empty dictionary means null object
			object = ObjectWrapper();
			return true;
		}

		auto type_it = data.data.find(uuid_to_base64_string(Property::property_schema_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_property)));
		VADON_ASSERT(type_it != data.data.end(), "Invalid data!");

		const TypeUUID object_type_uuid = std::get<::Vadon::Foundation::UUID>(type_it->second);
		const TypeID object_type_id = TypeRegistry::get_type_id(object_type_uuid);

		auto properties_it = data.data.find(uuid_to_base64_string(Property::property_schema_to_uuid(::Vadon::Foundation::DataObjectSchema::c_properties_property)));
		VADON_ASSERT(properties_it != data.data.end(), "Invalid data!");

		const VariantDictionary& property_data = *std::get<BoxedVariantDictionary>(properties_it->second);

		ObjectWrapper new_object = TypeRegistry::create_object(object_type_id);
		if (load_object_property_data(new_object, property_data) == false)
		{
			TypeRegistry::destroy_object(new_object);
			return false;
		}

		object = new_object;
		return true;
	}

	bool ObjectSerializer::load_object_property_data(ObjectWrapper& object, const VariantDictionary& property_data)
	{
		for (const auto& data_it : property_data.data)
		{
			PropertyUUID property_id;
			if (uuid_from_base64_string(data_it.first, property_id) == false)
			{
				return false;
			}
			TypeRegistry::set_property(object.get_data(), object.get_type(), property_id, data_it.second);
		}

		return true;
	}

	bool ObjectSerializer::store_object_data(const ObjectWrapper& object, VariantDictionary& data)
	{
		if (object.is_valid() == false)
		{
			// Nothing to save
			data.data.clear();
			return true;
		}

		{
			const std::string type_key = uuid_to_base64_string(Property::property_schema_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_property));
			const TypeUUID type_uuid = TypeRegistry::get_type_info(object.get_type()).id;

			data.data.insert(std::make_pair(type_key, type_uuid));
		}

		const std::string properties_key = uuid_to_base64_string(Property::property_schema_to_uuid(::Vadon::Foundation::DataObjectSchema::c_properties_property));
		VariantDictionary properties_dictionary;

		if (store_object_property_data(object, properties_dictionary) == false)
		{
			return false;
		}

		data.data.insert(std::make_pair(properties_key, Box(properties_dictionary)));

		return true;
	}

	bool ObjectSerializer::store_object_property_data(const ObjectWrapper& object, VariantDictionary& property_data)
	{
		// TODO: optimize by only saving values that changed from default?
		const PropertyInfoList property_info_list = TypeRegistry::get_type_properties(object.get_type());
		for (const PropertyInfo& current_property_info : property_info_list)
		{
			const std::string current_key = uuid_to_base64_string(current_property_info.base_info.id);

			const Variant property_value = TypeRegistry::get_property(object.get_data(), object.get_type(), current_property_info.base_info.id);

			property_data.data.insert(std::make_pair(current_key, property_value));
		}

		return true;
	}

	bool ObjectSerializer::serialize_property_data(Serializer& serializer, const PropertyInfo& property_info, Variant& property_data)
	{
		const ::Vadon::Foundation::Property::Category property_category = PropertyInfo::get_category(property_info.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			constexpr const char* key = ::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::TRIVIAL);
			const ::Vadon::Foundation::BaseType base_type = Vadon::Utilities::base_type_from_uuid(property_info.base_info.root_type);
			if (serialize_base_type<std::string_view>(serializer, base_type, key, property_data) == false)
			{
				return false;
			}
		}
		break;
		case ::Vadon::Foundation::Property::Category::ARRAY:			
		{
			constexpr const char* key = ::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::ARRAY);
			if (serialize_object_array_property(serializer, key, property_info, property_data) == false)
			{
				return false;
			}
		}
		break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
		{
			constexpr const char* key = ::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::DICTIONARY);
			if (serialize_object_dictionary_property(serializer, key, property_info, property_data) == false)
			{
				return false;
			}
		}
		break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
		{
			constexpr const char* key = ::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::OBJECT);
			if (serialize_object_subobject_property(serializer, key, property_info, property_data) == false)
			{
				return false;
			}
		}
			break;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			constexpr const char* key = ::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::TRIVIAL);
			if (serialize_base_type<std::string_view>(serializer, ::Vadon::Foundation::BaseType::UUID, key, property_data) == false)
			{
				return false;
			}
		}
		break;
		default:
			// Unsupported category!
			return false;
		}

		return true;
	}
}