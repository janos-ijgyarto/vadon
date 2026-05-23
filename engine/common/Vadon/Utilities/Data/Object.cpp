#include <Vadon/Utilities/Data/Object.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

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

	bool serialize_base_type(Vadon::Utilities::Serializer& serializer, ::Vadon::Foundation::BaseType base_type, std::string_view key, Vadon::Utilities::Variant& value)
	{
		using BaseType = ::Vadon::Foundation::BaseType;
		switch (base_type)
		{
		case BaseType::INT32:
			return serialize_base_type_impl<std::string_view, int>(serializer, key, value);
		case BaseType::UINT32:
			return serialize_base_type_impl<std::string_view, uint32_t>(serializer, key, value);
		case BaseType::FLOAT:
			return serialize_base_type_impl<std::string_view, float>(serializer, key, value);
		case BaseType::BOOL:
			return serialize_base_type_impl<std::string_view, bool>(serializer, key, value);
		case BaseType::STRING:
			return serialize_base_type_impl<std::string_view, std::string>(serializer, key, value);
		case BaseType::VECTOR2:
			return serialize_base_type_impl<std::string_view, Vadon::Math::Vector2>(serializer, key, value);
		case BaseType::VECTOR2I:
			return serialize_base_type_impl<std::string_view, Vadon::Math::Vector2i>(serializer, key, value);
		case BaseType::VECTOR3:
			return serialize_base_type_impl<std::string_view, Vadon::Math::Vector3>(serializer, key, value);
		case BaseType::VECTOR3I:
			return serialize_base_type_impl<std::string_view, Vadon::Math::Vector3i>(serializer, key, value);
		case BaseType::VECTOR4:
			return serialize_base_type_impl<std::string_view, Vadon::Math::Vector4>(serializer, key, value);
		case BaseType::COLORRGBA:
			return serialize_base_type_impl<std::string_view, Vadon::Math::ColorRGBA>(serializer, key, value);
		case BaseType::UUID:
			return serialize_base_type_impl<std::string_view, ::Vadon::Foundation::UUID>(serializer, key, value);
		default:
			return false;
		}
	}
}

namespace Vadon::Utilities
{
	Vadon::Foundation::UUID DataObject::deserialize_type_uuid(Serializer& serializer)
	{
		VADON_ASSERT(serializer.is_object() == true, "Serializer is in invalid state!");

		constexpr Vadon::Foundation::UUID type_property_uuid = Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_type_property);

		const Serializer::KeyVector keys = serializer.get_keys();
		for (const std::string& current_key : keys)
		{
			const Vadon::Foundation::UUID current_property_id = Utilities::parse_labeled_uuid(current_key);
			if (current_property_id == type_property_uuid)
			{
				Vadon::Foundation::UUID result;
				if (serializer.serialize(current_key, result) == Serializer::Result::SUCCESSFUL)
				{
					return result;
				}
			}
		}

		return Vadon::Foundation::UUID();
	}

	bool DataObject::serialize_object(Serializer& serializer, const Vadon::Foundation::UUID& type_uuid, ObjectPointer object_ptr)
	{
		constexpr Vadon::Foundation::UUID properties_property_uuid = Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property);
		if (serializer.is_reading() == true)
		{
			const Serializer::KeyVector keys = serializer.get_keys();
			for (const std::string& current_key : keys)
			{
				const Vadon::Foundation::UUID current_property_id = Utilities::parse_labeled_uuid(current_key);
				if (current_property_id == properties_property_uuid)
				{
					if (serializer.open_object(current_key) != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}

					if (serialize_object_properties(serializer, type_uuid, object_ptr) == false)
					{
						return false;
					}

					if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}
				}
			}
		}
		else
		{
			constexpr Vadon::Foundation::UUID type_property_uuid = Property::property_schema_to_uuid(Vadon::Foundation::DataObjectSchema::c_type_property);

			// FIXME: should we serialize with a label?
			{
				::Vadon::Foundation::UUID type_temp_uuid = type_uuid;
				if (serializer.serialize(type_property_uuid, type_temp_uuid) != Serializer::Result::SUCCESSFUL)
				{
					return false;
				}
			}

			if (serializer.open_object(properties_property_uuid) != Serializer::Result::SUCCESSFUL)
			{
				return false;
			}

			if (serialize_object_properties(serializer, type_uuid, object_ptr) == false)
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

	bool DataObject::serialize_object_properties(Serializer& serializer, const Vadon::Foundation::UUID& type_uuid, ObjectPointer object_ptr)
	{
		const TypeID type_id = TypeRegistry::get_type_id(type_uuid);

		if (serializer.is_reading() == true)
		{
			const Serializer::KeyVector keys = serializer.get_keys();
			for (const std::string& current_key : keys)
			{
				const Vadon::Foundation::UUID current_property_id = Utilities::parse_labeled_uuid(current_key);

				const PropertyInfo property_info = TypeRegistry::get_property_info(type_id, current_property_id);

				switch (property_info.get_category())
				{
				case PropertyCategory::TRIVIAL:
				{
					Variant temp_value;
					if (serialize_base_type(serializer, base_type_from_uuid(property_info.base_info.type), current_key, temp_value) == false)
					{
						return false;
					}
					TypeRegistry::set_property(object_ptr, type_id, current_property_id, temp_value);
				}
					break;
				case PropertyCategory::ARRAY:
				{
					const ::Vadon::Foundation::BaseType base_type = base_type_from_uuid(property_info.base_info.type);
					switch (base_type)
					{
					case ::Vadon::Foundation::BaseType::ARRAY:
					case ::Vadon::Foundation::BaseType::DICTIONARY:
						VADON_ERROR("Nested containers are not allowed!");
						return false;
					case ::Vadon::Foundation::BaseType::INVALID:
					{
						// Assume it's an array of objects
						const TypeID array_obj_type_id = TypeRegistry::get_type_id(property_info.base_info.type);
						if (serializer.open_array(current_key) != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}
						VariantArray object_array;
						object_array.data_type = TypeRegistry::get_type_id(TypeRegistryTrait<ObjectPointer>::get_type_uuid());
						const size_t array_size = serializer.get_array_size();
						for (size_t current_index = 0; current_index < array_size; ++current_index)
						{
							ObjectPointer current_obj = TypeRegistry::create_object(array_obj_type_id);
							object_array.data.push_back(current_obj);

							if (serializer.open_object(current_index) != Serializer::Result::SUCCESSFUL)
							{
								return false;
							}

							if (serialize_object_properties(serializer, property_info.base_info.type, current_obj) == false)
							{
								return false;
							}

							if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
							{
								return false;
							}
						}
						if (serializer.close_array() != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}

						// Set the property in the parent object
						// FIXME: this will require copying each object
						// The best solution would be to construct in-place and deserialize within the objects
						TypeRegistry::set_property(object_ptr, type_id, current_property_id, Box(object_array));

						// Clean up the temporary objects
						for (const Variant& current_obj_variant : object_array.data)
						{
							ObjectPointer current_obj = std::get<ObjectPointer>(current_obj_variant);
							TypeRegistry::destroy_object(array_obj_type_id, current_obj);
						}
					}
						break;
					default:
					{
						if (serializer.open_array(current_key) != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}
						VariantArray data_array;
						data_array.data_type = TypeRegistry::get_type_id(property_info.base_info.type);

						const size_t array_size = serializer.get_array_size();
						for (size_t current_index = 0; current_index < array_size; ++current_index)
						{
							Variant temp_value;
							if (serialize_base_type(serializer, base_type, current_key, temp_value) == false)
							{
								return false;
							}
							data_array.data.push_back(temp_value);
						}
						if (serializer.close_array() != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}

						TypeRegistry::set_property(object_ptr, type_id, current_property_id, Box(data_array));
					}
						break;
					}
				}
					break;
				case PropertyCategory::OBJECT:
				{
					if (serializer.open_object(current_key) != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}

					ObjectPointer sub_obj_ptr = std::get<ObjectPointer>(TypeRegistry::get_property(object_ptr, type_id, current_property_id));
					if (serialize_object_properties(serializer, property_info.base_info.type, sub_obj_ptr) == false)
					{
						return false;
					}

					if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}
				}
				break;
				}
			}
		}
		else
		{
			const PropertyInfoList property_info_list = TypeRegistry::get_type_properties(type_id);
			for (const PropertyInfo& current_property_info : property_info_list)
			{
				// FIXME: use labeled UUID?
				const std::string current_key = Vadon::Utilities::uuid_to_base64_string(current_property_info.base_info.id);
				switch (current_property_info.get_category())
				{
				case PropertyCategory::TRIVIAL:
				{
					Variant temp_value = TypeRegistry::get_property(object_ptr, type_id, current_property_info.base_info.id);
					if (serialize_base_type(serializer, base_type_from_uuid(current_property_info.base_info.type), current_key, temp_value) == false)
					{
						return false;
					}
				}
				break;
				case PropertyCategory::ARRAY:
				{
					const ::Vadon::Foundation::BaseType base_type = base_type_from_uuid(current_property_info.base_info.type);
					switch (base_type)
					{
					case ::Vadon::Foundation::BaseType::ARRAY:
					case ::Vadon::Foundation::BaseType::DICTIONARY:
						VADON_ERROR("Nested containers are not allowed!");
						return false;
					case ::Vadon::Foundation::BaseType::INVALID:
					{
						// Assume it's an array of objects
						if (serializer.open_array(current_key) != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}

						const BoxedVariantArray object_array = std::get<BoxedVariantArray>(TypeRegistry::get_property(object_ptr, type_id, current_property_info.base_info.id));
						for (size_t current_index = 0; current_index < object_array->data.size(); ++current_index)
						{
							if (serializer.open_object(current_index) != Serializer::Result::SUCCESSFUL)
							{
								return false;
							}

							ObjectPointer current_obj = std::get<ObjectPointer>(object_array->data[current_index]);
							if (serialize_object_properties(serializer, current_property_info.base_info.type, current_obj) == false)
							{
								return false;
							}

							if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
							{
								return false;
							}
						}
						if (serializer.close_array() != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}
					}
					break;
					default:
					{
						if (serializer.open_array(current_key) != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}

						BoxedVariantArray data_array = std::get<BoxedVariantArray>(TypeRegistry::get_property(object_ptr, type_id, current_property_info.base_info.id));

						for (size_t current_index = 0; current_index < data_array->data.size(); ++current_index)
						{
							Variant& temp_value = data_array->data[current_index];
							if (serialize_base_type(serializer, base_type, current_key, temp_value) == false)
							{
								return false;
							}
						}
						if (serializer.close_array() != Serializer::Result::SUCCESSFUL)
						{
							return false;
						}
					}
					break;
					}
				}
				break;
				case PropertyCategory::OBJECT:
				{
					if (serializer.open_object(current_key) != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}

					ObjectPointer sub_obj_ptr = std::get<ObjectPointer>(TypeRegistry::get_property(object_ptr, type_id, current_property_info.base_info.id));
					if (serialize_object_properties(serializer, current_property_info.base_info.type, sub_obj_ptr) == false)
					{
						return false;
					}

					if (serializer.close_object() != Serializer::Result::SUCCESSFUL)
					{
						return false;
					}
				}
				break;
				}
			}
		}

		return true;
	}
}