#include <Vadon/Utilities/Data/Object.hpp>

#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>

namespace
{
	Vadon::Utilities::Variant get_trivial_property_default_value(::Vadon::Foundation::BaseType base_type)
	{
		using BaseType = ::Vadon::Foundation::BaseType;
		switch (base_type)
		{
		case BaseType::INT32:
			return ::Vadon::Foundation::int32{ 0 };
		case BaseType::UINT32:
			return ::Vadon::Foundation::uint32{ 0 };
		case BaseType::FLOAT:
			return 0.0f;
		case BaseType::BOOL:
			return false;
		case BaseType::STRING:
			return std::string();
		case BaseType::VECTOR2:
			return Vadon::Math::Vector2_Zero;
		case BaseType::VECTOR2I:
			return Vadon::Math::Vector2i{ 0, 0 };
		case BaseType::VECTOR3:
			return Vadon::Math::Vector3_Zero;
		case BaseType::VECTOR3I:
			return Vadon::Math::Vector3i{ 0, 0, 0 };
		case BaseType::VECTOR4:
			return Vadon::Math::Vector4_Zero;
		case BaseType::COLORRGBA:
			return Vadon::Math::Color_Black;
		case BaseType::UUID:
			return ::Vadon::Foundation::UUID{};
		default:
			return Vadon::Utilities::Variant{};
		}
	}

	bool get_object_subobject_default_value_impl(Vadon::Utilities::TypeUUID object_type, bool allow_subclass, Vadon::Utilities::Variant& value)
	{
		if (((object_type.is_valid() == true) && (allow_subclass == true)) || (object_type.is_valid() == false))
		{
			// Just use a "null" object
			value = Vadon::Utilities::Box(Vadon::Utilities::VariantDictionary());
		}
		else if (object_type.is_valid() == true)
		{
			// Create a "default" instance of the object
			const Vadon::Utilities::TypeID object_type_id = Vadon::Utilities::TypeRegistry::get_type_id(object_type);
			Vadon::Utilities::DataObject data_object(object_type_id);
			if (data_object.default_initialize() == false)
			{
				return false;
			}

			value = Vadon::Utilities::Box(data_object.get_properties());
		}
		else
		{
			VADON_ERROR("Invalid parameters!");
			return false;
		}

		return true;
	}

	bool get_object_property_default_value(const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& value, bool shallow)
	{
		if (shallow == true)
		{
			// Use empty dictionary ("null" object)
			value = Vadon::Utilities::Box(Vadon::Utilities::VariantDictionary());
			return true;
		}

		// Check if the property uses ObjectWrapper
		const ::Vadon::Utilities::TypeUUID element_type = property_info.type_list[type_list_offset];
		if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_info.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return get_object_subobject_default_value_impl(Vadon::Utilities::TypeUUID{}, false, value);
			}
			else
			{
				// Constrain to the type specified in the type list
				const Vadon::Utilities::TypeUUID object_type = property_info.type_list[1];
				return get_object_subobject_default_value_impl(object_type, true, value);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is initialized as generic object
			return get_object_subobject_default_value_impl(Vadon::Utilities::TypeUUID{}, false, value);
		}
		else
		{
			// Initialize explicitly typed object (only this type)
			return get_object_subobject_default_value_impl(element_type, false, value);
		}
	}

	bool get_property_default_value(const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& value, bool shallow)
	{
		const ::Vadon::Foundation::UUID element_type = property_info.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = Vadon::Utilities::PropertyInfo::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			const ::Vadon::Foundation::BaseType base_type = Vadon::Utilities::base_type_from_uuid(element_type);
			value = get_trivial_property_default_value(base_type);
		}
		break;
		case ::Vadon::Foundation::Property::Category::ARRAY:
		{
			value = Vadon::Utilities::Box(Vadon::Utilities::VariantArray{});
		}
		break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
		{
			value = Vadon::Utilities::Box(Vadon::Utilities::VariantDictionary{});
		}
		break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
		{
			if (get_object_property_default_value(property_info, type_list_offset, value, shallow) == false)
			{
				return false;
			}
		}
		break;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			value = ::Vadon::Foundation::UUID{};
		}
		break;
		default:
			// Unsupported category!
			return false;
		}

		return true;
	}

	Vadon::Utilities::Variant query_object_array_nested_array_element(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, const Vadon::Utilities::Variant& /*array_element_value*/)
	{
		// FIXME: not supported yet (but it's possible now!)
		return Vadon::Utilities::Variant{};
	}

	Vadon::Utilities::Variant query_object_array_nested_dictionary_element(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, const Vadon::Utilities::Variant& /*array_element_value*/)
	{
		// FIXME: not supported yet (but it's possible now!)
		return Vadon::Utilities::Variant{};
	}

	Vadon::Utilities::Variant query_object_subobject_impl(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::Variant& value, const ::Vadon::Foundation::UUID& object_type, bool allow_subclass)
	{
		// Check if it's a generic or explicit object
		// FIXME: instead of creating a DataObject (which adds a copy), we should have a "view" implementation
		// which takes pointers/references to the underlying data and implements the DataObject API
		Vadon::Utilities::DataObject data_object;
		if (((object_type.is_valid() == true) && (allow_subclass == true)) || (object_type.is_valid() == false))
		{
			if (data_object.import_data(*std::get<Vadon::Utilities::BoxedVariantDictionary>(value)) == false)
			{
				return Vadon::Utilities::Variant();
			}
		}
		else if (object_type.is_valid() == true)
		{
			// Assume we are processing the object properties
			const Vadon::Utilities::TypeID object_type_id = Vadon::Utilities::TypeRegistry::get_type_id(object_type);
			VADON_ASSERT(object_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type!");
			data_object = Vadon::Utilities::DataObject(object_type_id);

			data_object.import_properties(*std::get<Vadon::Utilities::BoxedVariantDictionary>(value));
		}
		else
		{
			VADON_ERROR("Invalid parameters!");
		}

		return data_object.get_property(property_path);
	}

	Vadon::Utilities::Variant query_object_array_subobject_element(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, const Vadon::Utilities::Variant& array_element_value)
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
				return query_object_subobject_impl(property_path, array_element_value, ::Vadon::Foundation::UUID{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const ::Vadon::Utilities::TypeUUID object_type = property_info.type_list[object_type_offset];
				return query_object_subobject_impl(property_path, array_element_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return query_object_subobject_impl(property_path, array_element_value, ::Vadon::Foundation::UUID{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return query_object_subobject_impl(property_path, array_element_value, element_type, false);
		}
	}

	Vadon::Utilities::Variant query_object_array_property_value(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, const Vadon::Utilities::Variant& array_value)
	{
		VADON_ASSERT(property_path.front().is_array_index() == true, "Path must contain array index!");

		const Vadon::Utilities::BoxedVariantArray& array_data = std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
		const Vadon::Utilities::Variant& array_element_value = array_data->data[property_path.front().index];

		if (property_path.size() == 1)
		{
			// Return the element as-is
			return array_element_value;
		}

		const ::Vadon::Foundation::UUID element_type = property_info.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = Vadon::Utilities::PropertyInfo::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return query_object_array_nested_array_element(property_path.subspan(1), property_info, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return query_object_array_nested_dictionary_element(property_path.subspan(1), property_info, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return query_object_array_subobject_element(property_path.subspan(1), property_info, type_list_offset, array_element_value);
		default:
			VADON_ERROR("Invalid path!");
			return Vadon::Utilities::Variant();
		}
	}

	Vadon::Utilities::Variant query_object_dictionary_property_value(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, const Vadon::Utilities::Variant& /*property_value*/)
	{
		// TODO: implement dictionary support!
		return Vadon::Utilities::Variant();
	}

	Vadon::Utilities::Variant query_object_subobject_property(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, const Vadon::Utilities::Variant& property_value)
	{
		// Check if the property uses ObjectWrapper
		const Vadon::Utilities::TypeUUID element_type = property_info.type_list.front();
		if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_info.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return query_object_subobject_impl(property_path, property_value, Vadon::Utilities::TypeUUID{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const Vadon::Utilities::TypeUUID object_type = property_info.type_list[1];
				return query_object_subobject_impl(property_path, property_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return query_object_subobject_impl(property_path, property_value, Vadon::Utilities::TypeUUID{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return query_object_subobject_impl(property_path, property_value, element_type, false);
		}
	}

	Vadon::Utilities::Variant query_object_property_value(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, const Vadon::Utilities::Variant& property_value)
	{
		if (property_path.size() == 1)
		{
			// Whatever the property is, return as-is
			return property_value;
		}

		const ::Vadon::Foundation::Property::Category property_category = Vadon::Utilities::PropertyInfo::get_category(property_info.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return query_object_array_property_value(property_path.subspan(1), property_info, 1, property_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return query_object_dictionary_property_value(property_path.subspan(1), property_info, property_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return query_object_subobject_property(property_path.subspan(1), property_info, property_value);
		default:
			VADON_ERROR("Invalid path!");
			return Vadon::Utilities::Variant();
		}
	}

	enum class DataObjectPropertyEditMode
	{
		SET,
		ADD,
		REMOVE
	};

	void set_object_subobject_impl(Vadon::Utilities::PropertyPath property_path, Vadon::Utilities::Variant& value, const Vadon::Utilities::Variant& new_value, const ::Vadon::Foundation::UUID& object_type, bool allow_subclass, DataObjectPropertyEditMode edit_mode)
	{
		// Check if it's a generic or explicit object
		// FIXME: instead of creating a DataObject (which adds a copy), we should have a "view" implementation
		// which takes pointers/references to the underlying data and implements the DataObject API
		Vadon::Utilities::DataObject data_object;
		if (((object_type.is_valid() == true) && (allow_subclass == true)) || (object_type.is_valid() == false))
		{
			if (data_object.import_data(*std::get<Vadon::Utilities::BoxedVariantDictionary>(value)) == false)
			{
				return;
			}
		}
		else if (object_type.is_valid() == true)
		{
			// Assume we are processing the object properties
			const Vadon::Utilities::TypeID object_type_id = Vadon::Utilities::TypeRegistry::get_type_id(object_type);
			VADON_ASSERT(object_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type!");
			data_object = Vadon::Utilities::DataObject(object_type_id);

			data_object.import_properties(*std::get<Vadon::Utilities::BoxedVariantDictionary>(value));
		}
		else
		{
			VADON_ERROR("Invalid parameters!");
		}

		// FIXME: this in particular is very hacky, adds a lot of redundant copying and cleanup
		// Should replace this with a "view"
		switch (edit_mode)
		{
		case DataObjectPropertyEditMode::SET:
			data_object.set_property(property_path, new_value);
		break;
		case DataObjectPropertyEditMode::ADD:
			data_object.add_property(property_path, new_value);
		break;
		case DataObjectPropertyEditMode::REMOVE:
			data_object.remove_property(property_path);
		break;
		}
		data_object.set_property(property_path, new_value);
		value = Vadon::Utilities::Box(data_object.export_data());
	}
	
	void set_object_array_nested_array_element(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, Vadon::Utilities::Variant& /*array_element_value*/, const Vadon::Utilities::Variant& /*new_value*/, DataObjectPropertyEditMode /*edit_mode*/)
	{
		// FIXME: not supported yet (but it's possible now!)
	}

	void set_object_array_nested_dictionary_element(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, size_t /*type_list_offset*/, Vadon::Utilities::Variant& /*array_element_value*/, const Vadon::Utilities::Variant& /*new_value*/, DataObjectPropertyEditMode /*edit_mode*/)
	{
		// FIXME: not supported yet (but it's possible now!)
	}

	void set_object_array_subobject_element(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& array_element_value, const Vadon::Utilities::Variant& new_value, DataObjectPropertyEditMode edit_mode)
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
				set_object_subobject_impl(property_path, array_element_value, new_value, ::Vadon::Foundation::UUID{}, false, edit_mode);
			}
			else
			{
				// Constrain to the type specified in the type list
				const ::Vadon::Utilities::TypeUUID object_type = property_info.type_list[object_type_offset];
				set_object_subobject_impl(property_path, array_element_value, new_value, object_type, true, edit_mode);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			set_object_subobject_impl(property_path, array_element_value, new_value, ::Vadon::Foundation::UUID{}, false, edit_mode);
		}
		else
		{
			set_object_subobject_impl(property_path, array_element_value, new_value, element_type, false, edit_mode);
		}
	}

	void set_object_array_property_value(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, size_t type_list_offset, Vadon::Utilities::Variant& array_value, const Vadon::Utilities::Variant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		VADON_ASSERT(property_path.front().is_array_index() == true, "Path must contain array index!");
		Vadon::Utilities::BoxedVariantArray& array_data = std::get<Vadon::Utilities::BoxedVariantArray>(array_value);

		if (property_path.size() == 1)
		{
			switch (edit_mode)
			{
			case DataObjectPropertyEditMode::SET:
			{
				// Set element here
				array_data->data[property_path.front().index] = new_value;				
			}
			break;
			case DataObjectPropertyEditMode::ADD:
			{
				// Insert new element
				array_data->data.insert(array_data->data.begin() + property_path.front().index, new_value);				
			}
			break;
			case DataObjectPropertyEditMode::REMOVE:
			{
				// Remove existing element
				array_data->data.erase(array_data->data.begin() + property_path.front().index);
			}
			break;
			}
			return;
		}

		// Path goes deeper, so we know it must point to an existing array element
		Vadon::Utilities::Variant& array_element_value = array_data->data[property_path.front().index];

		const ::Vadon::Foundation::UUID element_type = property_info.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = Vadon::Utilities::PropertyInfo::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			set_object_array_nested_array_element(property_path.subspan(1), property_info, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			set_object_array_nested_dictionary_element(property_path.subspan(1), property_info, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
			set_object_array_subobject_element(property_path.subspan(1), property_info, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		default:
			VADON_ERROR("Invalid path!");
			return;
		}
	}

	void set_object_dictionary_property_value(Vadon::Utilities::PropertyPath /*property_path*/, const Vadon::Utilities::PropertyInfo& /*property_info*/, Vadon::Utilities::Variant& /*property_value*/, const Vadon::Utilities::Variant& /*new_value*/, DataObjectPropertyEditMode /*edit_mode*/)
	{
		// TODO: implement dictionary support!
	}

	void set_object_subobject_property(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value, const Vadon::Utilities::Variant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		// Check if the property uses ObjectWrapper
		const Vadon::Utilities::TypeUUID element_type = property_info.type_list.front();
		if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_info.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				set_object_subobject_impl(property_path, property_value, new_value, Vadon::Utilities::TypeUUID{}, false, edit_mode);
			}
			else
			{
				// Constrain to the type specified in the type list
				const Vadon::Utilities::TypeUUID object_type = property_info.type_list[1];
				set_object_subobject_impl(property_path, property_value, new_value, object_type, true, edit_mode);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is processed as generic object
			set_object_subobject_impl(property_path, property_value, new_value, Vadon::Utilities::TypeUUID{}, false, edit_mode);
		}
		else
		{
			// Process explicitly typed object (only this type)
			set_object_subobject_impl(property_path, property_value, new_value, element_type, false, edit_mode);
		}
	}

	void set_object_property_value(Vadon::Utilities::PropertyPath property_path, const Vadon::Utilities::PropertyInfo& property_info, Vadon::Utilities::Variant& property_value, const Vadon::Utilities::Variant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		if (property_path.size() == 1)
		{
			switch (edit_mode)
			{
			case DataObjectPropertyEditMode::SET:
			case DataObjectPropertyEditMode::ADD:
			{
				// Set the value here
				property_value = new_value;
				return;
			}
			default:
				VADON_ERROR("Invalid edit mode!");
				return;
			}
		}

		const ::Vadon::Foundation::Property::Category property_category = Vadon::Utilities::PropertyInfo::get_category(property_info.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			set_object_array_property_value(property_path.subspan(1), property_info, 1, property_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			set_object_dictionary_property_value(property_path.subspan(1), property_info, property_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
			set_object_subobject_property(property_path.subspan(1), property_info, property_value, new_value, edit_mode);
			break;
		default:
			VADON_ERROR("Invalid path!");
			return;
		}
	}

	void set_object_instance_property_value(Vadon::Utilities::PropertyPath property_path, Vadon::Utilities::TypeID object_type, void* object_ptr, const Vadon::Utilities::Variant& value, DataObjectPropertyEditMode edit_mode)
	{
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");
		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const Vadon::Utilities::PropertyInfo property_info = Vadon::Utilities::TypeRegistry::get_property_info(object_type, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return;
		}

		// We get the "root" property data from the object, then use the DataObject path, then write it back into the object
		// FIXME: not very efficient, but it works for now
		Vadon::Utilities::Variant root_property_data = Vadon::Utilities::TypeRegistry::get_property(object_ptr, object_type, property_path.front().uuid);
		set_object_property_value(property_path, property_info, root_property_data, value, edit_mode);

		Vadon::Utilities::TypeRegistry::set_property(object_ptr, object_type, property_path.front().uuid, root_property_data);
	}
}

namespace Vadon::Utilities
{
	bool DataObject::default_initialize()
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		const PropertyInfoList property_info_list = TypeRegistry::get_type_properties(m_type_id);

		for (const PropertyInfo current_property_info : property_info_list)
		{	
			Variant property_value;
			if (get_property_default_value(current_property_info, 0, property_value, false) == false)
			{
				return false;
			}

			m_properties.data.insert(std::make_pair(uuid_to_base64_string(current_property_info.base_info.id), property_value));
		}

		return true;
	}

	Variant DataObject::get_property(const PropertyUUID& property_id) const
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(Utilities::TypeRegistry::get_property_info(m_type_id, property_id).base_info.is_valid() == true, "Property not found in object!");

		const std::string property_id_key = Utilities::uuid_to_base64_string(property_id);
		auto property_it = m_properties.data.find(property_id_key);
		if (property_it != m_properties.data.end())
		{
			return property_it->second;
		}

		// FIXME: look up default value from 
		return Variant();
	}

	void DataObject::set_property(const PropertyUUID& property_id, const Variant& value)
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(Utilities::TypeRegistry::get_property_info(m_type_id, property_id).base_info.is_valid() == true, "Property not found in object!");

		const std::string property_id_key = Utilities::uuid_to_base64_string(property_id);
		m_properties.data.insert(std::make_pair(property_id_key, value));
	}

	Variant DataObject::get_property(PropertyPath property_path) const
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");

		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const PropertyInfo property_info = TypeRegistry::get_property_info(m_type_id, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return Variant();
		}

		Variant property_value = get_property(property_path.front().uuid);
		return query_object_property_value(property_path, property_info, property_value);
	}

	void DataObject::set_property(PropertyPath property_path, const Variant& value)
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");

		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const PropertyInfo property_info = TypeRegistry::get_property_info(m_type_id, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return;
		}

		auto property_it = m_properties.data.find(uuid_to_base64_string(property_path.front().uuid));

		// When using set_property with path, it must be for existing data!
		// To add data that did not exist, use add_property
		VADON_ASSERT(property_it != m_properties.data.end(), "Property data not found!");

		set_object_property_value(property_path, property_info, property_it->second, value, DataObjectPropertyEditMode::SET);
	}

	void DataObject::add_property(PropertyPath property_path, const Variant& value)
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");

		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const PropertyInfo property_info = TypeRegistry::get_property_info(m_type_id, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return;
		}

		auto property_it = m_properties.data.find(uuid_to_base64_string(property_path.front().uuid));
		if (property_it == m_properties.data.end())
		{
			// First create the property
			// NOTE: this time we create it "shallow" because we only intend to add the properties along the path,
			// so we don't want sub-objects to be fully filled in
			Variant property_value;
			if (get_property_default_value(property_info, 0, property_value, true) == false)
			{
				VADON_ERROR("Failed to initialize property!");
				return;
			}

			property_it = m_properties.data.insert(std::make_pair(uuid_to_base64_string(property_path.front().uuid), property_value)).first;
		}

		set_object_property_value(property_path, property_info, property_it->second, value, DataObjectPropertyEditMode::ADD);
	}

	void DataObject::remove_property(PropertyPath property_path)
	{
		VADON_ASSERT(is_valid_type(), "Object is not set to valid type!");
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");

		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const PropertyInfo property_info = TypeRegistry::get_property_info(m_type_id, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return;
		}

		auto property_it = m_properties.data.find(uuid_to_base64_string(property_path.front().uuid));

		// When using remove_property with path, it must be for existing data!
		// To add data that did not exist, use add_property
		VADON_ASSERT(property_it != m_properties.data.end(), "Property data not found!");

		if (property_path.size() == 1)
		{
			// Simply erase the member here
			m_properties.data.erase(property_it);
			return;
		}

		set_object_property_value(property_path, property_info, property_it->second, Variant(), DataObjectPropertyEditMode::REMOVE);
	}

	Variant DataObject::get_object_property(PropertyPath property_path, TypeID object_type, void* object_ptr)
	{
		VADON_ASSERT(property_path.empty() == false, "Path is empty!");
		VADON_ASSERT(property_path.front().is_object_key(), "Path to object property must start with property key!");

		const PropertyInfo property_info = TypeRegistry::get_property_info(object_type, property_path.front().uuid);
		if (property_info.base_info.is_valid() == false)
		{
			VADON_ERROR("Cannot find property!");
			return Variant();
		}

		// We get the "root" property data from the object, then use the DataObject path
		// FIXME: not very efficient, but it works for now
		const Variant root_property_data = TypeRegistry::get_property(object_ptr, object_type, property_path.front().uuid);
		return query_object_property_value(property_path, property_info, root_property_data);
	}

	void DataObject::set_object_property(PropertyPath property_path, TypeID object_type, void* object_ptr, const Variant& value)
	{
		set_object_instance_property_value(property_path, object_type, object_ptr, value, DataObjectPropertyEditMode::SET);
	}

	void DataObject::add_object_property(PropertyPath property_path, TypeID object_type, void* object_ptr, const Variant& value)
	{
		set_object_instance_property_value(property_path, object_type, object_ptr, value, DataObjectPropertyEditMode::ADD);
	}

	void DataObject::remove_object_property(PropertyPath property_path, TypeID object_type, void* object_ptr)
	{
		set_object_instance_property_value(property_path, object_type, object_ptr, Variant{}, DataObjectPropertyEditMode::REMOVE);
	}

	VariantDictionary DataObject::export_data() const
	{
		VariantDictionary data;

		// NOTE: invalid object should be serialized as empty dictionary
		if (is_valid_type())
		{
			const std::string type_entry_key = Utilities::uuid_to_base64_string(Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_property.id));
			data.data.insert(std::make_pair(type_entry_key, Utilities::TypeRegistry::get_type_info(m_type_id).id));

			const std::string properties_entry_key = Utilities::uuid_to_base64_string(Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_properties_property.id));
			data.data.insert(std::make_pair(properties_entry_key, Box(m_properties)));
		}

		return data;
	}

	bool DataObject::import_data(const VariantDictionary& data)
	{
		if (data.data.empty() == false)
		{
			const std::string type_entry_key = Utilities::uuid_to_base64_string(Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_property.id));
			auto type_entry_it = data.data.find(type_entry_key);
			VADON_ASSERT(type_entry_it != data.data.end(), "Invalid data");

			m_type_id = Utilities::TypeRegistry::get_type_id(std::get<TypeUUID>(type_entry_it->second));

			const std::string properties_entry_key = Utilities::uuid_to_base64_string(Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_properties_property.id));
			auto properties_entry_it = data.data.find(properties_entry_key);
			VADON_ASSERT(properties_entry_it != data.data.end(), "Invalid data");

			import_properties(*std::get<BoxedVariantDictionary>(properties_entry_it->second));
		}
		else
		{
			// NOTE: invalid object should be serialized as empty dictionary
			m_type_id = TypeID::INVALID;
			m_properties.data.clear();
		}

		return true;
	}

	void DataObject::import_properties(const VariantDictionary& properties)
	{
		for (auto property_it = properties.data.begin(); property_it != properties.data.end(); ++property_it)
		{
			PropertyUUID property_uuid;
			if (Utilities::uuid_from_base64_string(property_it->first, property_uuid) == false)
			{
				continue;
			}

			const PropertyInfo property_info = TypeRegistry::get_property_info(m_type_id, property_uuid);
			if (property_info.base_info.is_valid() == false)
			{
				// TODO: add warning?
				continue;
			}

			m_properties.data.insert(std::make_pair(property_it->first, property_it->second));
		}
	}
}