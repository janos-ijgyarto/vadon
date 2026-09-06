#include <VadonEditor/Core/Data/Object.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

#include <QJsonArray>
#include <QJsonObject>

namespace
{
	bool serialize_object_trivial_property(QJsonValue& json_value, const QUuid& property_type, const QVariant& property_value)
	{
		// TODO: use property type?
		Q_UNUSED(property_type);
		json_value = VadonEditor::Utilities::save_variant_to_json(property_value);
		return true;
	}

	bool deserialize_object_trivial_property(const QJsonValueConstRef& json_value, const QUuid& property_type, QVariant& property_value)
	{
		const ::Vadon::Foundation::BaseType base_type = VadonEditor::Core::TypeData::get_base_type(property_type);
		const int base_qt_type = VadonEditor::Utilities::get_qt_typeid_from_base_type(base_type);

		property_value = VadonEditor::Utilities::get_variant_from_json(base_qt_type, json_value);

		return true;
	}

	bool serialize_object_subobject_impl(VadonEditor::Core::Application& application, QJsonValue& json_value, const QVariant& value, const QUuid& object_type, bool allow_subclass, bool labeled)
	{
		// Check if it's a generic or explicit object
		const QVariantMap object_dictionary = value.toMap();
		QJsonObject json_object;

		if (((VadonEditor::Utilities::is_uuid_valid(object_type) == true) && (allow_subclass == true)) || (VadonEditor::Utilities::is_uuid_valid(object_type) == false))
		{
			if (object_dictionary.isEmpty())
			{
				// If null, early out
				json_value = json_object;
				return true;
			}

			// FIXME: also validate when serializing?

			// FIXME2: allow for more "seamless" conversion from Map to Object
			// instead of having to "serialize twice"
			VadonEditor::Core::DataObject data_object(application);
			if (data_object.import_data(object_dictionary) == false)
			{
				return false;
			}

			if (data_object.serialize(json_object, labeled) == false)
			{
				return false;
			}
		}
		else if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
		{
			// Assume we are processing the object properties
			VadonEditor::Core::DataObject data_object(application);
			if (data_object.init_type(object_type) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::DataObject", "Failed to initialize type");
				return false;
			}

			data_object.load_properties(object_dictionary);

			if (data_object.serialize_properties(json_object, labeled) == false)
			{
				return false;
			}
		}
		else
		{
			Q_ASSERT_X(false, "serialize_object_subobject_impl", "Invalid parameters!");
		}

		json_value = json_object;
		return true;
	}

	bool get_object_subobject_default_value_impl(VadonEditor::Core::Application& application, const QUuid& object_type, bool allow_subclass, QVariant& value)
	{
		if (((VadonEditor::Utilities::is_uuid_valid(object_type) == true) && (allow_subclass == true)) || (VadonEditor::Utilities::is_uuid_valid(object_type) == false))
		{
			// Just use a "null" object
			// FIXME: add support on engine side for parsing "null" in JSON
			value = QVariantMap();
		}
		else if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
		{
			// Create a "default" instance of the object
			VadonEditor::Core::DataObject data_object(application);
			if (data_object.default_initialize(object_type) == false)
			{
				return false;
			}

			value = data_object.get_property_map();
		}
		else
		{
			Q_ASSERT_X(false, "get_object_subobject_default_value_impl", "Invalid parameters!");
			return false;
		}

		return true;
	}

	bool get_object_subobject_default_value(VadonEditor::Core::Application& application, const VadonEditor::Core::PropertyData& property_data, QVariant& value, bool shallow)
	{
		if (shallow == true)
		{
			// Use empty dictionary ("null" object)
			value = QVariantMap();
			return true;
		}

		// Check if the property uses ObjectWrapper
		const QUuid element_type = property_data.type_list.front();
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_data.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return get_object_subobject_default_value_impl(application, QUuid{}, false, value);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[1];
				return get_object_subobject_default_value_impl(application, object_type, true, value);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is serialized as generic object
			return get_object_subobject_default_value_impl(application, QUuid{}, false, value);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return get_object_subobject_default_value_impl(application, element_type, false, value);
		}
	}

	bool get_data_object_property_default_value(VadonEditor::Core::Application& application, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& value, bool shallow)
	{
		const QUuid element_type = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category property_category = VadonEditor::Core::PropertyData::get_category(element_type);
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			value = VadonEditor::Utilities::get_base_type_default_value(VadonEditor::Core::TypeData::get_base_type(property_data.get_root_type()));
		}
		break;
		case ::Vadon::Foundation::Property::Category::ARRAY:
		{
			value = QVariantList();
		}
		break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
		{
			value = QVariantMap();
		}
		break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
		{
			if (get_object_subobject_default_value(application, property_data, value, shallow) == false)
			{
				return false;
			}
		}
		break;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			value = VadonEditor::Utilities::get_base_type_default_value(::Vadon::Foundation::BaseType::UUID);
		}
		break;
		default:
			// Unsupported category!
			return false;
		}

		return true;
	}

	bool deserialize_object_subobject_impl(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, QVariant& value, const QUuid& object_type, bool allow_subclass)
	{
		// Check if it's a generic or explicit object
		QVariantHash object_dictionary;
		if (((VadonEditor::Utilities::is_uuid_valid(object_type) == true) && (allow_subclass == true)) || (VadonEditor::Utilities::is_uuid_valid(object_type) == false))
		{
			const QJsonObject json_object = json_value.toObject();
			if (json_object.isEmpty() == true)
			{
				value = QVariantMap();
			}

			if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
			{
				// Ensure that the deserialized type is compatible
				const QUuid type_uuid = VadonEditor::Core::DataObject::deserialize_object_type(json_object);

				if (application.get_project_manager().get_project_data_schema().is_base_of(object_type, type_uuid) == false)
				{
					return false;
				}
			}

			VadonEditor::Core::DataObject data_object(application);
			if (data_object.deserialize(json_object) == false)
			{
				return false;
			}

			value = data_object.export_data();
		}
		else if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
		{
			// Assume we are processing the object properties
			VadonEditor::Core::DataObject data_object(application);
			if (data_object.init_type(object_type) == false)
			{
				return false;
			}

			if (data_object.deserialize_properties(json_value.toObject()) == false)
			{
				return false;
			}

			value = data_object.get_property_map();
		}
		else
		{
			Q_ASSERT_X(false, "deserialize_object_subobject_impl", "Invalid parameters!");
		}

		return true;
	}

	bool serialize_object_array_nested_array_element(QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return false;
	}

	bool deserialize_object_array_nested_array_element(const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return false;
	}

	bool serialize_object_array_nested_dictionary_element(QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return false;
	}

	bool deserialize_object_array_nested_dictionary_element(const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return false;
	}

	bool serialize_object_array_subobject_element(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value, bool labeled)
	{
		// Check if the array uses ObjectWrapper
		const QUuid element_type = property_data.type_list[type_list_offset];
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			const qsizetype object_type_offset = type_list_offset + 1;
			if (object_type_offset >= property_data.type_list.size())
			{
				// No type specified, so it's a generic object
				return serialize_object_subobject_impl(application, json_value, array_element_value, QUuid{}, false, labeled);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[object_type_offset];
				return serialize_object_subobject_impl(application, json_value, array_element_value, object_type, true, labeled);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is serialized as generic object
			return serialize_object_subobject_impl(application, json_value, array_element_value, QUuid{}, false, labeled);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return serialize_object_subobject_impl(application, json_value, array_element_value, element_type, false, labeled);
		}
	}

	bool deserialize_object_array_subobject_element(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value)
	{
		// Check if the array uses ObjectWrapper
		const QUuid element_type = property_data.type_list[type_list_offset];
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			const qsizetype object_type_offset = type_list_offset + 1;
			if (object_type_offset >= property_data.type_list.size())
			{
				// No type specified, so it's a generic object
				return deserialize_object_subobject_impl(application, json_value, array_element_value, QUuid{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[object_type_offset];
				return deserialize_object_subobject_impl(application, json_value, array_element_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return deserialize_object_subobject_impl(application, json_value, array_element_value, QUuid{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return deserialize_object_subobject_impl(application, json_value, array_element_value, element_type, false);
		}
	}

	bool serialize_object_array_resource_element(QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		const qsizetype resource_type_offset = type_list_offset + 1;
		Q_ASSERT_X(resource_type_offset < property_data.type_list.size(), "serialize_object_array_resource_element", "Missing resource type!");

		// Serialize the array element itself
		if (serialize_object_trivial_property(json_value, VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::UUID), array_element_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?
		//const Vadon::Utilities::TypeUUID resource_type_uuid = property_info.type_list[resource_type_offset];

		return true;
	}

	bool deserialize_object_array_resource_element(const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value)
	{
		const qsizetype resource_type_offset = type_list_offset + 1;
		Q_ASSERT_X(resource_type_offset < property_data.type_list.size(), "deserialize_object_array_resource_element", "Missing resource type!");

		// Serialize the array element itself
		if (deserialize_object_trivial_property(json_value, VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::UUID), array_element_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?
		//const Vadon::Utilities::TypeUUID resource_type_uuid = property_info.type_list[resource_type_offset];

		return true;
	}

	bool serialize_object_array_property_element(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value, bool labeled)
	{
		const QUuid element_type = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = VadonEditor::Core::PropertyData::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
			return serialize_object_trivial_property(json_value, element_type, array_element_value);
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return serialize_object_array_nested_array_element(json_value, property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return serialize_object_array_nested_dictionary_element(json_value, property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return serialize_object_array_subobject_element(application, json_value, property_data, type_list_offset, array_element_value, labeled);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return serialize_object_array_resource_element(json_value, property_data, type_list_offset, array_element_value);
		default:
			// Unsupported category!
			return false;
		}
	}

	bool deserialize_object_array_property_element(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value)
	{
		const QUuid element_type = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = VadonEditor::Core::PropertyData::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
			return deserialize_object_trivial_property(json_value, element_type, array_element_value);
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return deserialize_object_array_nested_array_element(json_value, property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return deserialize_object_array_nested_dictionary_element(json_value, property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return deserialize_object_array_subobject_element(application, json_value, property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return deserialize_object_array_resource_element(json_value, property_data, type_list_offset, array_element_value);
		default:
			// Unsupported category!
			return false;
		}
	}

	bool serialize_object_array_property(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value, bool labeled)
	{
		Q_ASSERT_X(property_data.type_list.front() == VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY), "serialize_object_array_property", "Invalid type!");

		const QVariantList array_value = property_value.toList();
		QJsonArray json_array;

		for (const QVariant& array_element_value : array_value)
		{
			QJsonValue current_array_value;
			if (serialize_object_array_property_element(application, current_array_value, property_data, 1, array_element_value, labeled) == false)
			{
				return false;
			}
			json_array.push_back(current_array_value);
		}

		json_value = json_array;
		return true;
	}

	bool deserialize_object_array_property(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value)
	{
		Q_ASSERT_X(property_data.type_list.front() == VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::ARRAY), "deserialize_object_array_property", "Invalid type!");

		const QJsonArray json_array = json_value.toArray();
		QVariantList array_value;

		for (const QJsonValueConstRef& current_array_value : json_array)
		{
			QVariant array_element_value;
			if (deserialize_object_array_property_element(application, current_array_value, property_data, 1, array_element_value) == false)
			{
				return false;
			}
			array_value.push_back(array_element_value);
		}

		property_value = array_value;
		return true;
	}

	bool serialize_object_dictionary_property(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value)
	{
		// FIXME: implement support for dictionary!
		Q_UNUSED(application);
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(property_value);
		return false;
	}

	bool deserialize_object_dictionary_property(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value)
	{
		// FIXME: implement support for dictionary!
		Q_UNUSED(application);
		Q_UNUSED(json_value);
		Q_UNUSED(property_data);
		Q_UNUSED(property_value);
		return false;
	}

	bool serialize_object_subobject_property(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value, bool labeled)
	{
		// Check if the property uses ObjectWrapper
		const QUuid element_type = property_data.type_list.front();
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_data.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return serialize_object_subobject_impl(application, json_value, property_value, QUuid{}, false, labeled);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[1];
				return serialize_object_subobject_impl(application, json_value, property_value, object_type, true, labeled);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is serialized as generic object
			return serialize_object_subobject_impl(application, json_value, property_value, QUuid{}, false, labeled);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return serialize_object_subobject_impl(application, json_value, property_value, element_type, false, labeled);
		}
	}

	bool deserialize_object_subobject_property(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value)
	{
		// Check if the property uses ObjectWrapper
		const QUuid element_type = property_data.type_list.front();
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_data.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return deserialize_object_subobject_impl(application, json_value, property_value, QUuid{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[1];
				return deserialize_object_subobject_impl(application, json_value, property_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is serialized as generic object
			return deserialize_object_subobject_impl(application, json_value, property_value, QUuid{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return deserialize_object_subobject_impl(application, json_value, property_value, element_type, false);
		}
	}

	bool serialize_object_resource_property(QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value)
	{
		Q_ASSERT_X(property_data.type_list.size() >= 2, "serialize_object_resource_property", "Missing resource type!");
		Q_UNUSED(property_data);

		// Serialize the UUID property itself
		if (serialize_object_trivial_property(json_value, VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::UUID), property_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?

		return true;
	}

	bool deserialize_object_resource_property(const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value)
	{
		Q_ASSERT_X(property_data.type_list.size() >= 2, "deserialize_object_resource_property", "Missing resource type!");
		Q_UNUSED(property_data);

		// Serialize the UUID property itself
		if (deserialize_object_trivial_property(json_value, VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::UUID), property_value) == false)
		{
			return false;
		}

		// TODO: find a way to validate that the resource ID points to a compatible resource?

		return true;
	}

	bool serialize_object_property_value(VadonEditor::Core::Application& application, QJsonValue& json_value, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value, bool labeled)
	{
		const ::Vadon::Foundation::Property::Category property_category = VadonEditor::Core::PropertyData::get_category(property_data.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
			return serialize_object_trivial_property(json_value, property_data.get_root_type(), property_value);
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return serialize_object_array_property(application, json_value, property_data, property_value, labeled);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return serialize_object_dictionary_property(application, json_value, property_data, property_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return serialize_object_subobject_property(application, json_value, property_data, property_value, labeled);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return serialize_object_resource_property(json_value, property_data, property_value);
		default:
			// Unsupported category!
			return false;
		}
	}

	bool deserialize_object_property_value(VadonEditor::Core::Application& application, const QJsonValueConstRef& json_value, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value)
	{
		const ::Vadon::Foundation::Property::Category property_category = VadonEditor::Core::PropertyData::get_category(property_data.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
			return deserialize_object_trivial_property(json_value, property_data.get_root_type(), property_value);
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return deserialize_object_array_property(application, json_value, property_data, property_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return deserialize_object_dictionary_property(application, json_value, property_data, property_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return deserialize_object_subobject_property(application, json_value, property_data, property_value);
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return deserialize_object_resource_property(json_value, property_data, property_value);
		default:
			// Unsupported category!
			return false;
		}
	}

	int get_data_object_property_qt_typeid(const VadonEditor::Core::PropertyData& property_data)
	{
		switch (VadonEditor::Core::PropertyData::get_category(property_data.get_root_type()))
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			const ::Vadon::Foundation::BaseType base_type = VadonEditor::Core::TypeData::get_base_type(property_data.get_root_type());
			return VadonEditor::Utilities::get_qt_typeid_from_base_type(base_type);
		}
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return QMetaType::Type::QVariantList;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return QMetaType::Type::QVariantMap;
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return QMetaType::Type::QVariantMap;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
			return QMetaType::Type::QUuid;
		default:
			// Unsupported category!
			return QMetaType::Type::UnknownType;
		}
	}

	QVariant query_object_subobject_impl(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const QVariant& value, const QUuid& object_type, bool allow_subclass)
	{
		// Check if it's a generic or explicit object
		// FIXME: instead of creating a DataObject (which adds a copy), we should have a "view" implementation
		// which takes pointers/references to the underlying data and implements the DataObject API
		VadonEditor::Core::DataObject data_object(application);
		if (((VadonEditor::Utilities::is_uuid_valid(object_type) == true) && (allow_subclass == true)) || (VadonEditor::Utilities::is_uuid_valid(object_type) == false))
		{
			if (data_object.import_data(value.toMap()) == false)
			{
				return QVariant();
			}
		}
		else if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
		{
			// Assume we are processing the object properties
			data_object.init_type(object_type);
			data_object.load_properties(value.toMap());
		}
		else
		{
			Q_ASSERT_X(false, "query_object_subobject_impl", "Invalid parameters!");
		}

		return data_object.get_property(property_path);
	}

	QVariant query_object_array_nested_array_element(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return QVariant{};
	}

	QVariant query_object_array_nested_dictionary_element(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		return QVariant{};
	}

	QVariant query_object_array_subobject_element(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_element_value)
	{
		// Check if the array uses ObjectWrapper
		const QUuid element_type = property_data.type_list[type_list_offset];
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			const qsizetype object_type_offset = type_list_offset + 1;
			if (object_type_offset >= property_data.type_list.size())
			{
				// No type specified, so it's a generic object
				return query_object_subobject_impl(application, property_path, array_element_value, QUuid{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[object_type_offset];
				return query_object_subobject_impl(application, property_path, array_element_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return query_object_subobject_impl(application, property_path, array_element_value, QUuid{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return query_object_subobject_impl(application, property_path, array_element_value, element_type, false);
		}
	}

	QVariant query_object_array_property_value(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, const QVariant& array_value)
	{
		Q_ASSERT_X(property_path.front().is_array_index() == true, "query_object_array_property_value", "Path must contain array index!");

		const QVariantList array_data = array_value.toList();
		const QVariant& array_element_value = array_data[property_path.front().index];

		if (property_path.size() == 1)
		{
			// Return the element as-is
			return array_element_value;
		}

		const QUuid element_type = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = VadonEditor::Core::PropertyData::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return query_object_array_nested_array_element(property_path.subspan(1), property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return query_object_array_nested_dictionary_element(property_path.subspan(1), property_data, type_list_offset, array_element_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return query_object_array_subobject_element(application, property_path.subspan(1), property_data, type_list_offset, array_element_value);
		default:
			Q_ASSERT_X(false, "query_object_array_property_value", "Invalid path!");
			return QVariant();
		}
	}

	QVariant query_object_dictionary_property_value(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value)
	{
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(property_value);
		// TODO: implement dictionary support!
		return QVariant();
	}

	QVariant query_object_subobject_property(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value)
	{
		// Check if the property uses ObjectWrapper
		const QUuid element_type = property_data.type_list.front();
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_data.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				return query_object_subobject_impl(application, property_path, property_value, QUuid{}, false);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[1];
				return query_object_subobject_impl(application, property_path, property_value, object_type, true);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			return query_object_subobject_impl(application, property_path, property_value, QUuid{}, false);
		}
		else
		{
			// Serialize explicitly typed object (only this type)
			return query_object_subobject_impl(application, property_path, property_value, element_type, false);
		}
	}

	QVariant query_object_property_value(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, const QVariant& property_value)
	{
		if (property_path.size() == 1)
		{
			// Whatever the property is, return as-is
			return property_value;
		}

		const ::Vadon::Foundation::Property::Category property_category = VadonEditor::Core::PropertyData::get_category(property_data.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			return query_object_array_property_value(application, property_path.subspan(1), property_data, 1, property_value);
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			return query_object_dictionary_property_value(property_path.subspan(1), property_data, property_value);
		case ::Vadon::Foundation::Property::Category::OBJECT:
			return query_object_subobject_property(application, property_path.subspan(1), property_data, property_value);
		default:
			Q_ASSERT_X(false, "query_object_property_value", "Invalid path!");
			return QVariant();
		}
	}

	enum class DataObjectPropertyEditMode
	{
		SET,
		ADD,
		REMOVE
	};

	void set_object_subobject_impl(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, QVariant& value, const QVariant& new_value, const QUuid& object_type, bool allow_subclass, DataObjectPropertyEditMode edit_mode)
	{
		// Check if it's a generic or explicit object
		// FIXME: instead of creating a DataObject (which adds a copy), we should have a "view" implementation
		// which takes pointers/references to the underlying data and implements the DataObject API
		VadonEditor::Core::DataObject data_object(application);
		if (((VadonEditor::Utilities::is_uuid_valid(object_type) == true) && (allow_subclass == true)) || (VadonEditor::Utilities::is_uuid_valid(object_type) == false))
		{
			if (data_object.import_data(value.toMap()) == false)
			{
				return;
			}
		}
		else if (VadonEditor::Utilities::is_uuid_valid(object_type) == true)
		{
			// Assume we are processing the object properties
			data_object.init_type(object_type);
			data_object.load_properties(value.toMap());
		}
		else
		{
			Q_ASSERT_X(false, "set_object_subobject_impl", "Invalid parameters!");
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
		value = data_object.export_data();
	}

	void set_object_array_nested_array_element(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		Q_UNUSED(new_value);
		Q_UNUSED(edit_mode);
	}

	void set_object_array_nested_dictionary_element(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		// FIXME: not supported yet (but it's possible now!)
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(type_list_offset);
		Q_UNUSED(array_element_value);
		Q_UNUSED(new_value);
		Q_UNUSED(edit_mode);
	}

	void set_object_array_subobject_element(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariant& array_element_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		// Check if the array uses ObjectWrapper
		const QUuid element_type = property_data.type_list[type_list_offset];
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			const qsizetype object_type_offset = type_list_offset + 1;
			if (object_type_offset >= property_data.type_list.size())
			{
				// No type specified, so it's a generic object
				set_object_subobject_impl(application, property_path, array_element_value, new_value, QUuid{}, false, edit_mode);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[object_type_offset];
				set_object_subobject_impl(application, property_path, array_element_value, new_value, object_type, true, edit_mode);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is deserialized as generic object
			set_object_subobject_impl(application, property_path, array_element_value, new_value, QUuid{}, false, edit_mode);
		}
		else
		{
			set_object_subobject_impl(application, property_path, array_element_value, new_value, element_type, false, edit_mode);
		}
	}

	void set_object_array_property_value(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset, QVariantList& array_data, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		Q_ASSERT_X(property_path.front().is_array_index() == true, "set_object_array_property_value", "Path must contain array index!");
		if (property_path.size() == 1)
		{
			switch (edit_mode)
			{
			case DataObjectPropertyEditMode::SET:
			{
				// Set element here
				array_data[property_path.front().index] = new_value;
			}
			break;
			case DataObjectPropertyEditMode::ADD:
			{
				// Insert new element
				array_data.insert(array_data.begin() + property_path.front().index, new_value);
			}
			break;
			case DataObjectPropertyEditMode::REMOVE:
			{
				// Remove existing element
				array_data.erase(array_data.begin() + property_path.front().index);
			}
			break;
			}
			return;
		}

		// Path goes deeper, so we know it must point to an existing array element
		QVariant& array_element_value = array_data[property_path.front().index];

		const QUuid element_type = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category element_category = VadonEditor::Core::PropertyData::get_category(element_type);
		switch (element_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
			set_object_array_nested_array_element(property_path.subspan(1), property_data, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			set_object_array_nested_dictionary_element(property_path.subspan(1), property_data, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
			set_object_array_subobject_element(application, property_path.subspan(1), property_data, type_list_offset, array_element_value, new_value, edit_mode);
			break;
		default:
			Q_ASSERT_X(false, "set_object_array_property_value", "Invalid path!");
			return;
		}
	}

	void set_object_dictionary_property_value(VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		Q_UNUSED(property_path);
		Q_UNUSED(property_data);
		Q_UNUSED(property_value);
		Q_UNUSED(new_value);
		Q_UNUSED(edit_mode);
		// TODO: implement dictionary support!
	}

	void set_object_subobject_property(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
	{
		// Check if the property uses ObjectWrapper
		const QUuid element_type = property_data.type_list.front();
		if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
		{
			// Check whether an explicit type is provided
			if (property_data.type_list.size() < 2)
			{
				// No type specified, so it's a generic object
				set_object_subobject_impl(application, property_path, property_value, new_value, QUuid{}, false, edit_mode);
			}
			else
			{
				// Constrain to the type specified in the type list
				const QUuid object_type = property_data.type_list[1];
				set_object_subobject_impl(application, property_path, property_value, new_value, object_type, true, edit_mode);
			}
		}
		// FIXME: this is a bit convoluted, find a way to deduplicate this logic!
		else if (element_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid))
		{
			// DataObject is processed as generic object
			set_object_subobject_impl(application, property_path, property_value, new_value, QUuid{}, false, edit_mode);
		}
		else
		{
			// Process explicitly typed object (only this type)
			set_object_subobject_impl(application, property_path, property_value, new_value, element_type, false, edit_mode);
		}
	}

	void set_object_property_value(VadonEditor::Core::Application& application, VadonEditor::Core::PropertyPath property_path, const VadonEditor::Core::PropertyData& property_data, QVariant& property_value, const QVariant& new_value, DataObjectPropertyEditMode edit_mode)
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
				Q_ASSERT_X(false, "set_object_property_value", "Invalid edit mode!");
				return;
			}
		}

		const ::Vadon::Foundation::Property::Category property_category = VadonEditor::Core::PropertyData::get_category(property_data.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::ARRAY:
		{
			QVariantList array_data = property_value.toList();
			set_object_array_property_value(application, property_path.subspan(1), property_data, 1, array_data, new_value, edit_mode);
			property_value = array_data;
		}
			break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
			set_object_dictionary_property_value(property_path.subspan(1), property_data, property_value, new_value, edit_mode);
			break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
			set_object_subobject_property(application, property_path.subspan(1), property_data, property_value, new_value, edit_mode);
			break;
		default:
			Q_ASSERT_X(false, "set_object_property_value", "Invalid path!");
			return;
		}
	}
}

namespace VadonEditor::Core
{
	DataObject::DataObject(Application& application)
		: m_application(application)
	{
	}

	bool DataObject::init_type(const QUuid& type_id)
	{
		if (Utilities::is_uuid_valid(type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::init_type", "Invalid type ID");
			return false;
		}

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const TypeData* type_data = data_schema.find_type_data(type_id);
		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::init_type", "Cannot find type data");
			return false;
		}

		m_type_id = type_id;
		return true;
	}

	bool DataObject::default_initialize(const QUuid& type_id)
	{
		if (init_type(type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::default_initialize", "Failed to initialize type");
			return false;
		}

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const TypeData* type_data = data_schema.find_type_data(m_type_id);
		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::default_initialize", "Cannot find type data");
			return false;
		}

		m_properties.clear();

		const TypeData* current_type_data = type_data;
		while (current_type_data != nullptr)
		{
			for (auto property_it = current_type_data->properties.begin(); property_it != current_type_data->properties.end(); ++property_it)
			{
				const QString property_key_string = Utilities::uuid_to_base64_string(property_it.key());

				const Core::PropertyData& type_property_data = property_it.value();

				QVariant property_value;
				if (get_data_object_property_default_value(m_application, type_property_data, 0, property_value, false) == false)
				{
					return false;
				}

				m_properties[property_key_string] = property_value;
			}

			const QUuid base_uuid = Utilities::vadon_uuid_to_qt_uuid(current_type_data->info.base_id);
			if (Utilities::is_uuid_valid(base_uuid) == true)
			{
				current_type_data = data_schema.find_type_data(base_uuid);
				Q_ASSERT_X(current_type_data != nullptr, "VadonEditor::Core::DataObject::default_initialize", "Cannot find base type data");
			}
			else
			{
				break;
			}
		}

		return true;
	}

	bool DataObject::import_data(const QVariantMap& data_map)
	{
		const QUuid type_id = data_map[Utilities::uuid_to_base64_string(get_type_property_uuid())].toUuid();

		if (init_type(type_id) == false)
		{
			return false;
		}

		const QVariantMap properties = data_map[Utilities::uuid_to_base64_string(get_properties_property_uuid())].toMap();
		load_properties(properties);

		return true;
	}

	QVariantMap DataObject::export_data() const
	{
		if (is_valid() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::export_data", "Object not initialized!");
			return QVariantMap();
		}

		QVariantMap object_data_map;

		object_data_map[Utilities::uuid_to_base64_string(get_type_property_uuid())] = m_type_id;
		object_data_map[Utilities::uuid_to_base64_string(get_properties_property_uuid())] = m_properties;

		return object_data_map;
	}

	QVariant DataObject::get_property(const PropertyID& property_id) const
	{
		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_id));
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Core::DataObject::get_property", "Cannot find property data");

		return property_it.value();
	}

	QVariant DataObject::get_property_default_value(const PropertyID& property_id) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_id);
		if (property_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::get_property_default_value", "Cannot find property!");
			return QVariant();
		}

		QVariant property_default_value;
		if (get_data_object_property_default_value(m_application, *property_data, 0, property_default_value, false) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::get_property_default_value", "Failed to create default value!");
			return QVariant();
		}

		return property_default_value;
	}

	bool DataObject::has_property(const PropertyID& property_id) const
	{
		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_id));
		return property_it != m_properties.end();
	}

	void DataObject::set_property(const PropertyID& property_id, const QVariant& value)
	{
		internal_set_property(property_id, value, false);
	}

	QVariant DataObject::get_property(PropertyPath property_path) const
	{
		Q_ASSERT_X(is_valid(), "VadonEditor::Core::DataObject::get_property", "Object is not set to valid type!");
		Q_ASSERT_X(property_path.isEmpty() == false, "VadonEditor::Core::DataObject::get_property", "Path is empty!");

		Q_ASSERT_X(property_path.front().is_object_key(), "VadonEditor::Core::DataObject::get_property", "Path to object property must start with property key!");

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_path.front().uuid);
		if (property_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::get_property", "Cannot find property!");
			return QVariant();
		}

		QVariant property_value = get_property(property_path.front().uuid);
		return query_object_property_value(m_application, property_path, *property_data, property_value);
	}

	void DataObject::set_property(PropertyPath property_path, const QVariant& value)
	{
		Q_ASSERT_X(is_valid() == true, "VadonEditor::Core::DataObject::set_property", "Object is not set to valid type!");
		Q_ASSERT_X(property_path.empty() == false, "VadonEditor::Core::DataObject::set_property", "Path is empty!");

		Q_ASSERT_X(property_path.front().is_object_key(), "VadonEditor::Core::DataObject::set_property", "Path to object property must start with property key!");

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_path.front().uuid);
		if (property_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::set_property", "Cannot find property!");
			return;
		}

		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_path.front().uuid));

		// When using set_property with path, it must be for existing data!
		// To add data that did not exist, use add_property
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Core::DataObject::set_property", "Property data not found!");

		set_object_property_value(m_application, property_path, *property_data, property_it.value(), value, DataObjectPropertyEditMode::SET);
	}

	void DataObject::add_property(PropertyPath property_path, const QVariant& value)
	{
		Q_ASSERT_X(is_valid() == true, "VadonEditor::Core::DataObject::add_property", "Object is not set to valid type!");
		Q_ASSERT_X(property_path.empty() == false, "VadonEditor::Core::DataObject::set_property", "Path is empty!");

		Q_ASSERT_X(property_path.front().is_object_key(), "VadonEditor::Core::DataObject::add_property", "Path to object property must start with property key!");

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_path.front().uuid);
		if (property_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::add_property", "Cannot find property!");
			return;
		}

		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_path.front().uuid));
		if (property_it == m_properties.end())
		{
			// First create the property
			// NOTE: this time we create it "shallow" because we only intend to add the properties along the path,
			// so we don't want sub-objects to be fully filled in
			QVariant property_value;
			if (get_data_object_property_default_value(m_application, *property_data, 0, property_value, true) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::add_property", "Failed to initialize property!");
				return;
			}

			property_it = m_properties.insert(Utilities::uuid_to_base64_string(property_path.front().uuid), property_value);
		}

		set_object_property_value(m_application, property_path, *property_data, property_it.value(), value, DataObjectPropertyEditMode::ADD);
	}

	void DataObject::remove_property(PropertyPath property_path)
	{
		Q_ASSERT_X(is_valid() == true, "VadonEditor::Core::DataObject::remove_property", "Object is not set to valid type!");
		Q_ASSERT_X(property_path.empty() == false, "VadonEditor::Core::DataObject::remove_property", "Path is empty!");

		Q_ASSERT_X(property_path.front().is_object_key(), "VadonEditor::Core::DataObject::remove_property", "Path to object property must start with property key!");

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_path.front().uuid);
		if (property_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::remove_property", "Cannot find property!");
			return;
		}

		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_path.front().uuid));

		// When using remove_property with path, it must be for existing data!
		// To add data that did not exist, use add_property
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Core::DataObject::remove_property", "Property data not found!");

		if (property_path.size() == 1)
		{
			// Simply erase the member here
			m_properties.erase(property_it);
			return;
		}

		set_object_property_value(m_application, property_path, *property_data, property_it.value(), QVariant(), DataObjectPropertyEditMode::REMOVE);
	}

	bool DataObject::serialize(QJsonObject& root_obj, bool labeled) const
	{
		{
			const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_property.id);
			root_obj[Utilities::create_uuid_key_string(type_property_uuid, L"type", labeled)] = Utilities::uuid_to_base64_string(m_type_id);
		}

		QJsonObject properties_obj;
		if (serialize_properties(properties_obj, labeled) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize", "Failed to serialize properties");
			return false;
		}

		const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property.id);
		root_obj[Utilities::create_uuid_key_string(properties_property_uuid, L"properties", labeled)] = properties_obj;

		return true;
	}

	bool DataObject::deserialize(const QJsonObject& root_obj)
	{
		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_property.id);
		const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property.id);

		if (root_obj.isEmpty() == true)
		{
			// NOTE: null object, so we clear the dictionary and early out
			m_properties.clear();
			return true;
		}

		auto properties_obj_it = root_obj.end();
		for (auto property_it = root_obj.begin(); property_it != root_obj.end(); ++property_it)
		{
			const QUuid& current_property_id = Utilities::parse_labeled_uuid(property_it.key());
			if (current_property_id == get_type_property_uuid())
			{
				const QUuid type_id = Utilities::base64_string_to_uuid(property_it.value().toString());
				if (Utilities::is_uuid_valid(m_type_id) == true)
				{
					Q_ASSERT_X(m_type_id == type_id, "VadonEditor::Core::DataObject::deserialize", "Invalid type in object data");
					return false;
				}
				m_type_id = type_id;
			}
			else if (current_property_id == get_properties_property_uuid())
			{
				if (const QJsonValueConstRef property_obj_value = property_it.value(); property_obj_value.isObject())
				{
					properties_obj_it = property_it;
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize", "Invalid data");
					return false;
				}
			}
			else
			{
				// TODO: warn about unrecognized data?
			}
		}

		if (Utilities::is_uuid_valid(m_type_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize", "Failed to load type ID");
			return false;
		}

		if (properties_obj_it != root_obj.end())
		{
			if (deserialize_properties(properties_obj_it.value().toObject()) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize", "Failed to deserialized properties object");
				return false;
			}
		}
		else
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize", "Failed to load properties object");
			return false;
		}

		return true;
	}

	bool DataObject::serialize_properties(QJsonObject& properties_obj, bool labeled) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		QUuid current_type_uuid = m_type_id;
		while (Utilities::is_uuid_valid(current_type_uuid) == true)
		{
			const Core::TypeData* type_data = data_schema.find_type_data(current_type_uuid);
			Q_ASSERT_X(type_data != nullptr, "VadonEditor::Core::DataObject::serialize_properties", "Cannot find type data");

			for (auto property_it = type_data->properties.begin(); property_it != type_data->properties.end(); ++property_it)
			{
				const Core::PropertyData* type_property_data = type_data->find_property_data(property_it.key());

				auto property_value_it = m_properties.find(Utilities::uuid_to_base64_string(property_it.key()));
				if (property_value_it == m_properties.end())
				{
					// Object has no data for this property
					continue;
				}

				if (internal_serialize_property_data(properties_obj, *type_property_data, property_value_it.value(), labeled) == false)
				{
					return false;
				}
			}

			current_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_data->info.base_id);
		}

		return true;
	}

	bool DataObject::deserialize_properties(const QJsonObject& properties_obj)
	{
		Q_ASSERT_X(is_valid() == true, "VadonEditor::Core::DataObject::deserialize_properties", "Type ID is not valid");
		m_properties.clear();

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		for (auto property_it = properties_obj.begin(); property_it != properties_obj.end(); ++property_it)
		{
			const QUuid property_uuid = Utilities::parse_labeled_uuid(property_it.key());
			const Core::PropertyData* type_property_data = data_schema.find_type_property_data(m_type_id, property_uuid);

			if (type_property_data == nullptr)
			{
				// Ignore property
				qWarning() << "Stale property" << property_uuid << "loaded for object type" << m_type_id;
				continue;
			}

			QVariant property_value;
			if(deserialize_object_property_value(m_application, property_it.value(), *type_property_data, property_value) == false)
			{
				return false;
			}

			set_property(property_uuid, property_value);
		}

		return true;
	}

	bool DataObject::serialize_property_data(const QUuid& property_id, QJsonObject& property_obj, bool labeled) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* type_property_data = data_schema.find_type_property_data(m_type_id, property_id);
		Q_ASSERT_X(type_property_data != nullptr, "VadonEditor::Core::DataObject::serialize_property_data", "Cannot find property");

		auto property_value_it = m_properties.find(Utilities::uuid_to_base64_string(property_id));
		Q_ASSERT_X(property_value_it != m_properties.end(), "VadonEditor::Core::DataObject::serialize_properties", "Cannot find property value");

		return internal_serialize_property_data(property_obj, *type_property_data, property_value_it.value(), labeled);
	}

	void DataObject::load_properties(const QVariantMap& properties)
	{
		for (auto property_it = properties.begin(); property_it != properties.end(); ++property_it)
		{
			internal_set_property(Utilities::base64_string_to_uuid(property_it.key()), property_it.value(), true);
		}
	}

	QUuid DataObject::get_object_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_uuid);
	}

	QUuid DataObject::get_type_property_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_property.id);
	}
	
	QUuid DataObject::get_properties_property_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property.id);
	}

	QUuid DataObject::deserialize_object_type(const QJsonObject& object)
	{
		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_property.id);

		for (auto obj_it = object.begin(); obj_it != object.end(); ++obj_it)
		{
			const QUuid current_property_id = Utilities::parse_labeled_uuid(obj_it.key());
			if (current_property_id == type_property_uuid)
			{
				return Utilities::base64_string_to_uuid(obj_it.value().toString());
			}
		}

		return QUuid();
	}

	bool DataObject::serialize_property_value(Application& application, QJsonObject& data_obj, const PropertyData& property_data, QVariant& property_value)
	{
		const ::Vadon::Foundation::Property::Category property_category = PropertyData::get_category(property_data.type_list.front());
		switch (property_category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			QJsonValue json_value;
			if (serialize_object_trivial_property(json_value, property_data.get_root_type(), property_value) == false)
			{
				return false;
			}
			data_obj.insert(::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::TRIVIAL), json_value);
		}
		break;
		case ::Vadon::Foundation::Property::Category::ARRAY:
		{
			QJsonValue json_value;
			if (serialize_object_array_property(application, json_value, property_data, property_value, false) == false)
			{
				return false;
			}
			data_obj.insert(::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::ARRAY), json_value);
		}
		break;
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
		{
			QJsonValue json_value;
			if (serialize_object_dictionary_property(application, json_value, property_data, property_value) == false)
			{
				return false;
			}
			data_obj.insert(::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::DICTIONARY), json_value);
		}
		break;
		case ::Vadon::Foundation::Property::Category::OBJECT:
		{
			QJsonValue json_value;
			if (serialize_object_subobject_property(application, json_value, property_data, property_value, false) == false)
			{
				return false;
			}
			data_obj.insert(::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::OBJECT), json_value);
		}
		break;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			QJsonValue json_value;
			if (serialize_object_trivial_property(json_value, VadonEditor::Core::TypeData::get_base_type_uuid(::Vadon::Foundation::BaseType::UUID), property_value) == false)
			{
				return false;
			}
			data_obj.insert(::Vadon::Foundation::DataObjectSchema::get_path_serializer_key(::Vadon::Foundation::DataObjectSchema::PathSerializerKey::TRIVIAL), json_value);
		}
		break;
		default:
			// Unsupported category!
			return false;
		}

		return true;
	}

	bool DataObject::internal_serialize_property_data(QJsonObject& json_obj, const PropertyData& type_property_data, const QVariant& property_value, bool labeled) const
	{
		const QUuid property_id = Utilities::vadon_uuid_to_qt_uuid(type_property_data.info.id);
		const QString property_name = type_property_data.find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);

		const QString key_string = Utilities::create_uuid_key_string(property_id, property_name, labeled);

		QJsonValue json_value;
		if (serialize_object_property_value(m_application, json_value, type_property_data, property_value, labeled) == false)
		{
			return false;
		}

		json_obj[key_string] = json_value;
		return true;
	}

	void DataObject::internal_set_property(const PropertyID& property_id, const QVariant& value, bool ignore_deprecated)
	{
		Q_ASSERT_X(is_valid() == true, "VadonEditor::Core::DataObject::internal_set_property", "Cannot set property on uninitialized object");

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* property_data = data_schema.find_type_property_data(m_type_id, property_id);
		if (property_data == nullptr)
		{
			if (ignore_deprecated == true)
			{
				// Ignore property
				qWarning() << "Stale property" << property_id << "loaded for object type" << m_type_id;
			}
			else
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::internal_set_property", "Property not found in type!");
			}
			return;
		}

		// Validate the incoming data type against the data schema
		const int property_typeid = get_data_object_property_qt_typeid(*property_data);
		if (property_typeid != value.typeId())
		{
			const int original_type_id = property_typeid;
			const int new_value_type_id = value.typeId();
			qCritical() << "Mismatch in data type while setting property! (original:" << original_type_id << "new:" << new_value_type_id << ")";
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::set_property", "Property value type mismatch");
			return;
		}

		m_properties.insert(Utilities::uuid_to_base64_string(property_id), value);
	}
}