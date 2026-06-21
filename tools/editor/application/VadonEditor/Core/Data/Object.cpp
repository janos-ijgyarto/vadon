#include <VadonEditor/Core/Data/Object.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

#include <QJsonArray>
#include <QJsonObject>

namespace
{
	bool serialize_trivial_array_to_json(const QUuid& array_data_type, const QVariant& array_data, QJsonArray& json_array)
	{
		const ::Vadon::Foundation::BaseType base_type = VadonEditor::Core::TypeData::get_base_type(array_data_type);
		Q_ASSERT_X(base_type != ::Vadon::Foundation::BaseType::INVALID, "VadonEditor::Core::DataObject::serialize_trivial_array_to_json", "Invalid data type!");

		if (base_type == ::Vadon::Foundation::BaseType::ARRAY || base_type == ::Vadon::Foundation::BaseType::DICTIONARY)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_array_to_json", "Cannot nest containers directly!");
			return false;
		}

		const QVariantList array_data_list = array_data.toList();
		for (const QVariant& current_entry : array_data_list)
		{
			switch (base_type)
			{
			case ::Vadon::Foundation::BaseType::UUID:
				json_array.push_back(VadonEditor::Utilities::uuid_to_base64_string(current_entry.toUuid()));
				break;
			default:
				json_array.push_back(QJsonValue::fromVariant(current_entry));
			}
		}

		return true;
	}

	bool deserialize_trivial_array_from_json(const QUuid& array_data_type, const QJsonArray& json_array, QVariant& array_data)
	{
		const ::Vadon::Foundation::BaseType base_type = VadonEditor::Core::TypeData::get_base_type(array_data_type);
		Q_ASSERT_X(base_type != ::Vadon::Foundation::BaseType::INVALID, "VadonEditor::Core::DataObject::serialize_trivial_array_to_json", "Invalid data type!");

		if (base_type == ::Vadon::Foundation::BaseType::ARRAY || base_type == ::Vadon::Foundation::BaseType::DICTIONARY)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_array_to_json", "Cannot nest containers directly!");
			return false;
		}

		const int base_qt_type = VadonEditor::Utilities::get_qt_typeid_from_base_type(base_type);
		QVariantList value_list;
		for (QJsonValueConstRef current_entry_value : json_array)
		{
			value_list.push_back(VadonEditor::Utilities::get_variant_from_json(base_qt_type, current_entry_value));
		}
		array_data = value_list;

		return true;
	}
}

namespace VadonEditor::Core
{
	DataObject::DataObject(Application& application)
		: m_application(application)
	{
	}

	bool DataObject::initialize(const QUuid& type_id)
	{
		if (type_id.isNull() == true)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::initialize", "Invalid type ID");
			return false;
		}

		m_type_id = type_id;
		return internal_initialize();
	}

	bool DataObject::import_data(const QVariantMap& data_map)
	{
		const QUuid type_id = data_map[Utilities::uuid_to_base64_string(get_type_property_uuid())].toUuid();

		if (initialize(type_id) == false)
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
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Core::DataObject::get_property", "Cannot find property");

		return property_it.value();
	}

	void DataObject::set_property(const PropertyID& property_id, const QVariant& value)
	{
		// TODO: validate that property is set with a value compatible with the schema?
		auto property_it = m_properties.find(Utilities::uuid_to_base64_string(property_id));
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Core::DataObject::set_property", "Cannot find property");
		if (property_it.value().typeId() != value.typeId())
		{
			const int original_type_id = property_it.value().typeId();
			const int new_value_type_id = value.typeId();
			qCritical() << "Mismatch in data type while setting property! (original:" << original_type_id << "new:" << new_value_type_id << ")";
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::set_property", "Property value type mismatch");
			return;
		}

		property_it.value() = value;
	}

	bool DataObject::serialize(QJsonObject& root_obj) const
	{
		{
			const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_type_property.id);
			root_obj[Utilities::serialize_labeled_uuid("type", type_property_uuid)] = Utilities::uuid_to_base64_string(m_type_id);
		}

		QJsonObject properties_obj;
		if (serialize_properties(properties_obj) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize", "Failed to serialize properties");
			return false;
		}

		const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::DataObjectSchema::c_properties_property.id);
		root_obj[Utilities::serialize_labeled_uuid("properties", properties_property_uuid)] = properties_obj;

		return true;
	}

	bool DataObject::deserialize(const QJsonObject& root_obj)
	{
		auto properties_obj_it = root_obj.end();
		for (auto property_it = root_obj.begin(); property_it != root_obj.end(); ++property_it)
		{
			const QUuid& current_property_id = Utilities::parse_labeled_uuid(property_it.key());
			if (current_property_id == get_type_property_uuid())
			{
				const QUuid type_id = Utilities::base64_string_to_uuid(property_it.value().toString());
				if (m_type_id.isNull() == false)
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

		if (m_type_id.isNull() == true)
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

	bool DataObject::serialize_properties(QJsonObject& properties_obj) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		QUuid current_type_uuid = m_type_id;
		while (current_type_uuid.isNull() == false)
		{
			const Core::TypeData* type_data = data_schema.find_type_data(current_type_uuid);
			Q_ASSERT_X(type_data != nullptr, "VadonEditor::Core::DataObject::serialize_properties", "Cannot find type data");

			for (auto property_it = type_data->properties.begin(); property_it != type_data->properties.end(); ++property_it)
			{
				const Core::PropertyData* type_property_data = type_data->find_property_data(property_it.key());

				auto property_value_it = m_properties.find(Utilities::uuid_to_base64_string(property_it.key()));
				Q_ASSERT_X(property_value_it != m_properties.end(), "VadonEditor::Core::DataObject::serialize_properties", "Cannot find property value");

				if (internal_serialize_property_data(*type_property_data, property_value_it.value(), properties_obj) == false)
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
		Q_ASSERT_X(m_type_id.isNull() == false, "VadonEditor::Core::DataObject::deserialize_properties", "Type ID is not valid");
		if (internal_initialize() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Failed to initialize");
			return false;
		}

		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		
		for (auto property_it = properties_obj.begin(); property_it != properties_obj.end(); ++property_it)
		{
			const QUuid property_uuid = Utilities::parse_labeled_uuid(property_it.key());
			const Core::PropertyData* type_property_data = data_schema.find_type_property_data(m_type_id, property_uuid);
						
			const PropertyCategory category = type_property_data->get_category();
			switch (category)
			{
			case PropertyCategory::TRIVIAL:
			case PropertyCategory::RESOURCE:
			{
				const ::Vadon::Foundation::BaseType base_type = category != PropertyCategory::RESOURCE ? TypeData::get_base_type(type_property_data->get_data_type()) : ::Vadon::Foundation::BaseType::UUID;
				const int base_qt_type = Utilities::get_qt_typeid_from_base_type(base_type);

				set_property(property_uuid, Utilities::get_variant_from_json(base_qt_type, property_it.value()));
			}
			break;
			case PropertyCategory::TRIVIAL_ARRAY:
			{
				if (const QJsonValueConstRef array_value_ref = property_it.value(); array_value_ref.isArray())
				{
					QVariant array_data;
					if (deserialize_trivial_array_from_json(type_property_data->get_data_type(), array_value_ref.toArray(), array_data) == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Failed to load array property");
						return false;
					}
					set_property(property_uuid, array_data);
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
					return false;
				}
			}
				break;
			case PropertyCategory::GENERIC_OBJECT_ARRAY:
			{
				if (const QJsonValueConstRef obj_array_value_ref = property_it.value(); obj_array_value_ref.isArray())
				{
					const QJsonArray json_array = obj_array_value_ref.toArray();

					QVariantList object_array;
					for (const QJsonValueConstRef current_array_value : json_array)
					{
						if (current_array_value.isObject())
						{
							QVariant object_data;
							if (deserialize_generic_object_from_json(current_array_value.toObject(), object_data) == false)
							{
								Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to load object property");
								return false;
							}
							object_array.push_back(object_data);
						}
						else
						{
							Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
							return false;
						}
					}

					set_property(property_uuid, object_array);
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
					return false;
				}
			}
				break;
			case PropertyCategory::TYPED_OBJECT_ARRAY:
			{
				const TypeData* array_obj_type_data = data_schema.find_type_data(type_property_data->get_data_type());

				if (array_obj_type_data == nullptr)
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Cannot find type data");
					return false;
				}

				if (const QJsonValueConstRef obj_array_value_ref = property_it.value(); obj_array_value_ref.isArray())
				{
					const QJsonArray json_array = obj_array_value_ref.toArray();

					QVariantList object_array;
					for (const QJsonValueConstRef current_array_value : json_array)
					{
						if (current_array_value.isObject())
						{
							QVariant object_data;
							if (deserialize_typed_object_from_json(array_obj_type_data, current_array_value.toObject(), object_data) == false)
							{
								Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to load object property");
								return false;
							}
							object_array.push_back(object_data);
						}
						else
						{
							Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
							return false;
						}
					}

					set_property(property_uuid, object_array);
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
					return false;
				}
			}
				break;
			case PropertyCategory::GENERIC_OBJECT:
			{
				if (const QJsonValueConstRef object_ref = property_it.value(); object_ref.isObject())
				{
					QVariant object_data;
					if (deserialize_generic_object_from_json(object_ref.toObject(), object_data) == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to load object property");
						return false;
					}
					set_property(property_uuid, object_data);
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
					return false;
				}
			}
				break;
			case PropertyCategory::TYPED_OBJECT:
			{
				if (const QJsonValueConstRef object_properties_ref = property_it.value(); object_properties_ref.isObject())
				{
					const Core::TypeData* sub_object_type_data = data_schema.find_type_data(type_property_data->get_data_type());
					QVariant object_data;
					if (deserialize_typed_object_from_json(sub_object_type_data, object_properties_ref.toObject(), object_data) == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to load object property");
						return false;
					}

					set_property(property_uuid, object_data);
				}
				else
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_properties", "Invalid data");
					return false;
				}
			}
				break;
			}
		}

		return true;
	}

	bool DataObject::serialize_property_data(const QUuid& property_id, QJsonObject& property_obj) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const PropertyData* type_property_data = data_schema.find_type_property_data(m_type_id, property_id);
		Q_ASSERT_X(type_property_data != nullptr, "VadonEditor::Core::DataObject::serialize_property_data", "Cannot find property");

		auto property_value_it = m_properties.find(Utilities::uuid_to_base64_string(property_id));
		Q_ASSERT_X(property_value_it != m_properties.end(), "VadonEditor::Core::DataObject::serialize_properties", "Cannot find property value");

		return internal_serialize_property_data(*type_property_data, property_value_it.value(), property_obj);
	}

	void DataObject::load_properties(const QVariantMap& properties)
	{
		for (auto property_it = properties.begin(); property_it != properties.end(); ++property_it)
		{
			auto internal_property_it = m_properties.find(property_it.key());
			if (internal_property_it != m_properties.end())
			{
				internal_property_it.value() = property_it.value();
			}
			else
			{
				qWarning() << "Cannot find property" << property_it.key() << "in object" << m_type_id.toString();
			}
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

	bool DataObject::internal_initialize()
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const TypeData* type_data = data_schema.find_type_data(m_type_id);

		if (type_data == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::internal_initialize", "Cannot find type data");
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
				const PropertyCategory category = type_property_data.get_category();
				switch (category)
				{
				case PropertyCategory::TRIVIAL:
				{
					m_properties[property_key_string] = Utilities::get_base_type_default_value(TypeData::get_base_type(type_property_data.get_data_type()));
				}
					break;
				case PropertyCategory::RESOURCE:
				{
					m_properties[property_key_string] = Utilities::get_base_type_default_value(::Vadon::Foundation::BaseType::UUID);
				}
					break;
				case PropertyCategory::TRIVIAL_ARRAY:
				case PropertyCategory::GENERIC_OBJECT_ARRAY:
				case PropertyCategory::TYPED_OBJECT_ARRAY:
				{
					m_properties[property_key_string] = QVariantList();
				}
					break;
				case PropertyCategory::GENERIC_OBJECT:
				{
					m_properties[property_key_string] = QVariantMap();
				}
					break;
				case PropertyCategory::TYPED_OBJECT:
				{
					DataObject sub_object(m_application);
					sub_object.m_type_id = type_property_data.get_data_type();
					if (sub_object.internal_initialize() == false)
					{
						Q_ASSERT_X(false, "VadonEditor::Core::DataObject::internal_initialize", "Failed to initialize sub-object");
						return false;
					}

					m_properties[property_key_string] = sub_object.m_properties;
				}
					break;
				}
			}

			const QUuid base_uuid = Utilities::vadon_uuid_to_qt_uuid(current_type_data->info.base_id);
			if (base_uuid.isNull() == false)
			{
				current_type_data = data_schema.find_type_data(base_uuid);
				Q_ASSERT_X(current_type_data != nullptr, "VadonEditor::Core::DataObject::initialize", "Cannot find base type data");
			}
			else
			{
				break;
			}
		}

		return true;
	}

	bool DataObject::internal_serialize_property_data(const PropertyData& type_property_data, const QVariant& property_value, QJsonObject& json_obj) const
	{
		const DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();

		const QUuid property_id = Utilities::vadon_uuid_to_qt_uuid(type_property_data.info.id);
		const QString property_name = type_property_data.find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);

		QString key_string;
		if (property_name.isEmpty() == false)
		{
			key_string = Utilities::serialize_labeled_uuid(property_name, property_id);
		}
		else
		{
			key_string = Utilities::uuid_to_base64_string(property_id);
		}

		const PropertyCategory category = type_property_data.get_category();
		switch (category)
		{
		case PropertyCategory::TRIVIAL:
		case PropertyCategory::RESOURCE:
			json_obj[key_string] = Utilities::save_variant_to_json(property_value);
			break;
		case PropertyCategory::TRIVIAL_ARRAY:
		{
			QJsonArray json_array;
			if (serialize_trivial_array_to_json(type_property_data.get_data_type(), property_value, json_array) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to save array property");
				return false;
			}
			json_obj[key_string] = json_array;
		}
		break;
		case PropertyCategory::GENERIC_OBJECT_ARRAY:
		{
			QJsonArray json_array;
			const QVariantList object_list = property_value.toList();

			for (const QVariant& current_object_data : object_list)
			{
				QJsonObject json_object;
				if (serialize_generic_object_to_json(current_object_data, json_object) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to save object array property");
					return false;
				}
				json_array.push_back(json_object);
			}
			json_obj[key_string] = json_array;
		}
		break;
		case PropertyCategory::TYPED_OBJECT_ARRAY:
		{
			const Core::TypeData* sub_object_type_data = data_schema.find_type_data(type_property_data.get_data_type());

			QJsonArray json_array;
			const QVariantList object_list = property_value.toList();
			for (const QVariant& current_object_data : object_list)
			{
				QJsonObject json_object;
				if (serialize_typed_object_to_json(sub_object_type_data, current_object_data, json_object) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to save object array property");
					return false;
				}
				json_array.push_back(json_object);
			}
			json_obj[key_string] = json_array;
		}
		break;
		case VadonEditor::Core::PropertyCategory::GENERIC_OBJECT:
		{
			QJsonObject json_object;
			if (serialize_generic_object_to_json(property_value, json_object) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to save object property");
				return false;
			}

			json_obj[key_string] = json_object;
		}
		break;
		case VadonEditor::Core::PropertyCategory::TYPED_OBJECT:
		{
			const Core::TypeData* sub_object_type_data = data_schema.find_type_data(type_property_data.get_data_type());

			QJsonObject json_object;
			if (serialize_typed_object_to_json(sub_object_type_data, json_object, json_object) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to save object property");
				return false;
			}

			json_obj[key_string] = json_object;
		}
		break;
		}

		return true;
	}

	bool DataObject::serialize_generic_object_to_json(const QVariant& object_data, QJsonObject& json_object) const
	{
		DataObject data_object(m_application);
		if (data_object.import_data(object_data.toMap()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_generic_object_to_json", "Failed to prepare object");
			return false;
		}

		if (data_object.serialize(json_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_properties", "Failed to serialize object");
			return false;
		}

		return true;
	}

	bool DataObject::deserialize_generic_object_from_json(const QJsonObject& json_object, QVariant& object_data) const
	{
		DataObject data_object(m_application);
		if (data_object.deserialize(json_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_generic_object_from_json", "Failed to deserialize object");
			return false;
		}

		object_data = data_object.export_data();
		return true;
	}

	bool DataObject::serialize_typed_object_to_json(const TypeData* type_data, const QVariant& object_data, QJsonObject& json_object) const
	{
		DataObject data_object(m_application);
		data_object.m_type_id = Utilities::vadon_uuid_to_qt_uuid(type_data->info.id);
		data_object.m_properties = object_data.toMap();

		if (data_object.serialize_properties(json_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::serialize_typed_object_to_json", "Failed to serialize object");
			return false;
		}

		return true;
	}

	bool DataObject::deserialize_typed_object_from_json(const TypeData* type_data, const QJsonObject& json_object, QVariant& object_data) const
	{
		DataObject data_object(m_application);
		data_object.m_type_id = Utilities::vadon_uuid_to_qt_uuid(type_data->info.id);

		if (data_object.deserialize_properties(json_object) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Core::DataObject::deserialize_typed_object_from_json", "Failed to deserialize object");
			return false;
		}

		object_data = data_object.m_properties;
		return true;
	}
}