#include <VadonEditor/Core/Project/DataSchema.hpp>

#include <VadonEditor/Core/Project/Project.hpp>

#include <QDir>
#include <QFile>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// FIXME: move these to shared utilities?
namespace
{
	QUuid vadon_uuid_to_qt_uuid(const ::Vadon::Foundation::UUID& uuid)
	{
		// NOTE: Qt defaults to Big Endian, worth taking a second look?
		return QUuid::fromBytes(uuid.data, QSysInfo::Endian::LittleEndian);
	}

	::Vadon::Foundation::UUID qt_uuid_to_vadon_uuid(const QUuid& uuid)
	{
		// NOTE: Qt defaults to Big Endian, worth taking a second look?
		::Vadon::Foundation::UUID vadon_uuid;
		const QUuid::Id128Bytes uuid_bytes = uuid.toBytes(QSysInfo::Endian::LittleEndian);
		memcpy(vadon_uuid.data, &uuid_bytes, ::Vadon::Foundation::UUID::c_uuid_width);

		return vadon_uuid;
	}

	QUuid json_string_to_uuid(const QJsonValue& uuid_string_value)
	{
		QString uuid_string = uuid_string_value.toString();
		if (uuid_string.isEmpty())
		{
			return QUuid();
		}
		QByteArray uuid_arr = QByteArray::fromBase64(uuid_string.toUtf8());
		return QUuid::fromBytes(uuid_arr);
	}

	QString uuid_to_json_string(const QUuid& uuid)
	{
		if (uuid.isNull() == true)
		{
			return "";
		}
		QUuid::Id128Bytes uuid_bytes = uuid.toBytes(QSysInfo::LittleEndian);
		return QString(QByteArray(QByteArrayView(uuid_bytes)).toBase64());
	}
}

namespace VadonEditor::Core
{
	void DataSchema::register_type(const::Vadon::Foundation::TypeInfo& type_info)
	{
		Q_ASSERT_X(type_info.id.is_valid() == true, "VadonEditor::Core::DataSchema::register_type", "Type not registered!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_info.id);
		if (m_types.find(qt_type_uuid) != m_types.end())
		{
			qWarning() << "Type already registered!";
			return;
		}

		TypeData type_data;
		type_data.info = type_info;

		m_types.insert(qt_type_uuid, type_data);
		m_type_list.push_back(type_info.id);
	}

	::Vadon::Foundation::TypeInfo DataSchema::get_type_info(const ::Vadon::Foundation::UUID& type_uuid) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_info", "Invalid type UUID!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_info", "Type not registered!");
			return ::Vadon::Foundation::TypeInfo{};
		}

		return type_it->info;
	}

	::Vadon::Foundation::UUID DataSchema::get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_property_uuid", "Invalid type UUID!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_property_uuid", "Type not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		return type_it->property_list[property_index];
	}

	void DataSchema::set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_type_metadata", "Invalid type UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::set_type_metadata", "Invalid key!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);
		
		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_type_metadata", "Type not registered!");
			return;
		}

		type_it.value().metadata.insert(QString(key), QString(value).toUtf8());
	}

	const char* DataSchema::get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_metadata", "Invalid type UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::get_type_metadata", "Invalid key!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_metadata", "Type not registered!");
			return nullptr;
		}

		auto metadata_it = type_it->metadata.find(QString(key));
		if (metadata_it == type_it->metadata.end())
		{
			return nullptr;
		}

		return metadata_it->constData();
	}

	void DataSchema::register_property(const::Vadon::Foundation::UUID& type_uuid, const::Vadon::Foundation::Property& property)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::register_property", "Invalid type UUID!");
		Q_ASSERT_X(property.is_valid() == true, "VadonEditor::Core::DataSchema::register_property", "Invalid property info!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_metadata", "Type not registered!");
			return;
		}

		const QUuid qt_property_uuid = vadon_uuid_to_qt_uuid(property.id);
		if (type_it->properties.find(qt_property_uuid) != type_it->properties.end())
		{
			qWarning() << "Property already registered!";
			return;
		}

		PropertyData property_data;
		property_data.info = property;

		type_it->properties.insert(qt_property_uuid, property_data);
		type_it->property_list.push_back(property.id);
	}

	void DataSchema::set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid property UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid key!");
		
		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_property_metadata", "Type not registered!");
			return;
		}

		const QUuid qt_property_uuid = vadon_uuid_to_qt_uuid(property_uuid);
		auto property_it = type_it->properties.find(qt_property_uuid);

		if (property_it == type_it->properties.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_property_metadata", "Property not registered!");
			return;
		}

		property_it->metadata.insert(QString(key), QString(value).toUtf8());
	}

	const char* DataSchema::get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid property UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid key!");

		const QUuid qt_type_uuid = vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_metadata", "Type not registered!");
			return nullptr;
		}

		const QUuid qt_property_uuid = vadon_uuid_to_qt_uuid(property_uuid);
		auto property_it = type_it->properties.find(qt_property_uuid);

		if (property_it == type_it->properties.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_metadata", "Property not registered!");
			return nullptr;
		}

		auto metadata_it = property_it->metadata.find(key);
		if (metadata_it == property_it->metadata.end())
		{
			return nullptr;
		}

		return metadata_it->constData();
	}

	bool DataSchema::save_schema(const QString& schema_file_path)
	{
		QJsonObject data_schema_root;

		QJsonArray type_list_array;
		for (auto type_it = m_types.begin(); type_it != m_types.end(); ++type_it)
		{
			const TypeData& current_type_data = type_it.value();

			QJsonObject current_type_object;
			{
				QJsonObject type_info_object;

				const QUuid type_uuid = vadon_uuid_to_qt_uuid(current_type_data.info.id);
				const QUuid base_uuid = vadon_uuid_to_qt_uuid(current_type_data.info.base_id);

				type_info_object["id"] = uuid_to_json_string(type_uuid);
				type_info_object["base"] = uuid_to_json_string(base_uuid);
				// TODO: any other info?

				current_type_object["info"] = type_info_object;
			}

			{
				QJsonObject type_metadata_object;
				for (auto metadata_it = current_type_data.metadata.begin(); metadata_it != current_type_data.metadata.end(); ++metadata_it)
				{
					type_metadata_object.insert(metadata_it.key(), metadata_it.value().constData());
				}
				current_type_object["metadata"] = type_metadata_object;
			}

			{
				QJsonArray type_properties_array;
				for (auto property_it = current_type_data.properties.begin(); property_it != current_type_data.properties.end(); ++property_it)
				{
					const PropertyData& current_property_data = property_it.value();
					QJsonObject property_object;

					property_object["id"] = uuid_to_json_string(property_it.key());
					property_object["type"] = uuid_to_json_string(vadon_uuid_to_qt_uuid(current_property_data.info.type));

					QJsonObject property_metadata_object;
					for (auto metadata_it = current_property_data.metadata.begin(); metadata_it != current_property_data.metadata.end(); ++metadata_it)
					{
						property_metadata_object.insert(metadata_it.key(), metadata_it.value().constData());
					}

					property_object["metadata"] = property_metadata_object;

					type_properties_array.push_back(property_object);
				}
				current_type_object["properties"] = type_properties_array;
			}

			type_list_array.push_back(current_type_object);
		}

		data_schema_root["type_list"] = type_list_array;

		// Make sure path exists
		const QDir project_dir;
		const QFileInfo schema_file_info(schema_file_path);
		if (project_dir.mkpath(schema_file_info.absolutePath()) == false)
		{
			qCritical() << "Failed to create data schema directory!";
			return false;
		}

		QFile schema_file(schema_file_path);
		if (schema_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to write data schema file!";
			return false;
		}

		schema_file.write(QJsonDocument(data_schema_root).toJson());
		schema_file.close();

		return true;
	}

	bool DataSchema::load_schema(const QString& schema_file_path)
	{
		Q_ASSERT_X(schema_file_path.isEmpty() == false, "DataSchema::load_schema", "Schema path must not be empty!");

		QFile load_file(schema_file_path);

		if (load_file.open(QIODevice::ReadOnly) == false)
		{
			qWarning("Failed to open schema file.");
			return false;
		}

		// TODO: move this to its own object!
		QByteArray schema_data = load_file.readAll();

		// TODO: check error 
		QJsonDocument schema_document(QJsonDocument::fromJson(schema_data));
		if (schema_document.isNull() == true)
		{
			qWarning("Schema file is not valid JSON.");
			return false;
		}

		const QJsonObject& root_obj = schema_document.object();
		if (const QJsonValue type_arr_value = root_obj["type_list"]; type_arr_value.isArray())
		{
			m_types.clear();

			const QJsonArray& type_array = type_arr_value.toArray();
			for (const QJsonValue& current_type : type_array)
			{
				TypeData new_type_data;

				QUuid new_type_uuid;
				const QJsonObject& type_object = current_type.toObject();
				if (const QJsonValue type_info_value = type_object["info"]; type_info_value.isObject())
				{
					const QJsonObject type_info = type_info_value.toObject();
					if (const QJsonValue type_uuid_value = type_info["id"]; type_uuid_value.isString())
					{
						new_type_uuid = json_string_to_uuid(type_uuid_value);
						new_type_data.info.id = qt_uuid_to_vadon_uuid(new_type_uuid);
					}
					else
					{
						qCritical() << "Invalid schema format: cannot find type ID!";
						return false;
					}
					if (const QJsonValue base_uuid_value = type_info["base"]; base_uuid_value.isString())
					{
						new_type_data.info.base_id = qt_uuid_to_vadon_uuid(json_string_to_uuid(base_uuid_value));
					}
					else
					{
						qCritical() << "Invalid schema format: cannot find base ID!";
						return false;
					}
					// TODO: any other data?
				}
				else
				{
					qCritical() << "Invalid schema format: cannot find info in type!";
					return false;
				}

				if (const QJsonValue type_metadata_value = type_object["metadata"]; type_metadata_value.isObject())
				{
					const QJsonObject type_metadata = type_metadata_value.toObject();
					for (auto metadata_it = type_metadata.begin(); metadata_it != type_metadata.end(); ++metadata_it)
					{
						new_type_data.metadata.insert(metadata_it.key(), metadata_it.value().toString().toUtf8());
					}
				}

				if (const QJsonValue type_properties_value = type_object["properties"]; type_properties_value.isArray())
				{
					const QJsonArray type_properties = type_properties_value.toArray();
					for (const QJsonValue& current_property_value : type_properties)
					{
						const QJsonObject current_property_obj = current_property_value.toObject();

						QUuid property_uuid;
						PropertyData new_property_data;

						if (const QJsonValue property_id_value = current_property_obj["id"]; property_id_value.isString())
						{
							property_uuid = json_string_to_uuid(property_id_value);
							new_property_data.info.id = qt_uuid_to_vadon_uuid(property_uuid);
						}
						else
						{
							qCritical() << "Invalid schema format: cannot find property ID!";
							return false;
						}

						if (const QJsonValue property_type_value = current_property_obj["type"]; property_type_value.isString())
						{
							new_property_data.info.type = qt_uuid_to_vadon_uuid(json_string_to_uuid(property_type_value));
						}
						else
						{
							qCritical() << "Invalid schema format: cannot find property type!";
							return false;
						}

						if (const QJsonValue property_metadata_value = current_property_obj["metadata"]; property_metadata_value.isObject())
						{
							const QJsonObject property_metadata = property_metadata_value.toObject();
							for (auto metadata_it = property_metadata.begin(); metadata_it != property_metadata.end(); ++metadata_it)
							{
								new_property_data.metadata.insert(metadata_it.key(), metadata_it.value().toString().toUtf8());
							}
						}

						new_type_data.properties.insert(property_uuid, new_property_data);
						new_type_data.property_list.push_back(new_property_data.info.id);
					}

					new_type_data.info.property_count = new_type_data.properties.size();
				}
				else
				{
					qCritical() << "Invalid schema format: cannot find properties in type!";
					return false;
				}

				m_types.insert(new_type_uuid, new_type_data);
				m_type_list.push_back(new_type_data.info.id);
			}
		}
		else
		{
			qCritical() << "Invalid schema format: cannot find type list!";
			return false;
		}

		return true;
	}
}