#include <VadonEditor/Core/Project/DataSchema.hpp>

#include <QFile>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
	QUuid json_string_to_uuid(const QJsonValue& uuid_string_value)
	{
		QString uuid_string = uuid_string_value.toString();
		QByteArray uuid_arr = QByteArray::fromBase64(uuid_string.toUtf8());
		return QUuid::fromBytes(uuid_arr);
	}
}

namespace VadonEditor::Core
{
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

				const QJsonObject& type_object = current_type.toObject();
				if (const QJsonValue type_base_info_value = type_object["base_info"]; type_base_info_value.isObject())
				{
					const QJsonObject type_base_info = type_base_info_value.toObject();
					if (const QJsonValue type_uuid_value = type_base_info["type_uuid"]; type_uuid_value.isString())
					{
						new_type_data.id = json_string_to_uuid(type_uuid_value);
					}
				}
				else
				{
					qCritical() << "Invalid schema format: cannot find base info in type!";
					return false;
				}

				if (const QJsonValue type_metadata_value = type_object["metadata"]; type_metadata_value.isObject())
				{
					const QJsonObject type_metadata = type_metadata_value.toObject();
					if (const QJsonValue type_name = type_metadata["name"]; type_name.isString())
					{
						new_type_data.name = type_name.toString();
					}
				}

				if (const QJsonValue type_properties_value = type_object["properties"]; type_properties_value.isArray())
				{
					const QJsonArray type_properties = type_properties_value.toArray();
					for (const QJsonValue& current_property_value : type_properties)
					{
						PropertyData new_property_data;
						const QJsonObject current_property_obj = current_property_value.toObject();
						if (const QJsonValue property_id_value = current_property_obj["id"]; property_id_value.isString())
						{
							new_property_data.id = json_string_to_uuid(property_id_value);
						}
						if (const QJsonValue property_type_value = current_property_obj["type"]; property_type_value.isString())
						{
							new_property_data.type = json_string_to_uuid(property_type_value);
						}

						new_type_data.properties.push_back(new_property_data);
					}
				}
				else
				{
					qCritical() << "Invalid schema format: cannot find properties in type!";
					return false;
				}

				m_types.push_back(new_type_data);
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