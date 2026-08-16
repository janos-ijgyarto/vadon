#include <VadonEditor/Core/Data/Schema.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <QDir>
#include <QFile>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QStandardItemModel>

// FIXME: move these to shared utilities?
namespace
{
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
		if (VadonEditor::Utilities::is_uuid_valid(uuid) == false)
		{
			return "";
		}
		QUuid::Id128Bytes uuid_bytes = uuid.toBytes();
		return QString(QByteArray(QByteArrayView(uuid_bytes)).toBase64());
	}

	QStandardItem* create_type_tree_standard_item(const QString& label)
	{
		QStandardItem* new_item = new QStandardItem(label);
		new_item->setCheckable(false);
		new_item->setEditable(false);
		new_item->setDragEnabled(false);

		return new_item;
	}

	QStandardItem* find_type_tree_standard_item(QStandardItem* root, const QUuid& type_uuid)
	{
		for (int current_row = 0; current_row < root->rowCount(); ++current_row)
		{
			QStandardItem* current_child = root->child(current_row);
			const QUuid current_child_type_uuid = current_child->data(static_cast<int>(VadonEditor::Core::TypeTreeDataRole::TYPE_UUID)).toUuid();
			if (current_child_type_uuid == type_uuid)
			{
				return current_child;
			}
			else
			{
				QStandardItem* found_item = find_type_tree_standard_item(current_child, type_uuid);
				if (found_item != nullptr)
				{
					return found_item;
				}
			}
		}

		return nullptr;
	}
}

namespace VadonEditor::Core
{
	void DataSchema::TypeMetadataRegistry::register_type(const::Vadon::Foundation::TypeInfo& type_info)
	{
		Q_ASSERT_X(type_info.id.is_valid() == true, "VadonEditor::Core::DataSchema::register_type", "Type not registered!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_info.id);
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

	::Vadon::Foundation::TypeInfo DataSchema::TypeMetadataRegistry::get_type_info(const ::Vadon::Foundation::UUID& type_uuid) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_info", "Invalid type UUID!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_info", "Type not registered!");
			return ::Vadon::Foundation::TypeInfo{};
		}

		return type_it->info;
	}

	::Vadon::Foundation::UUID DataSchema::TypeMetadataRegistry::get_type_property_uuid(const ::Vadon::Foundation::UUID& type_uuid, size_t property_index) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_property_uuid", "Invalid type UUID!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_property_uuid", "Type not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		return type_it->property_list[property_index];
	}

	void DataSchema::TypeMetadataRegistry::set_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key, const char* value)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_type_metadata", "Invalid type UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::set_type_metadata", "Invalid key!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);
		
		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_type_metadata", "Type not registered!");
			return;
		}

		type_it.value().metadata.insert(QString(key), QString(value).toUtf8());
	}

	const char* DataSchema::TypeMetadataRegistry::get_type_metadata(const ::Vadon::Foundation::UUID& type_uuid, const char* key) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_type_metadata", "Invalid type UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::get_type_metadata", "Invalid key!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
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

	void DataSchema::TypeMetadataRegistry::register_property(const::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::Property& property, const ::Vadon::Foundation::UUID* type_list)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::register_property", "Invalid type UUID!");
		Q_ASSERT_X(property.is_valid() == true, "VadonEditor::Core::DataSchema::register_property", "Invalid property info!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_type_metadata", "Type not registered!");
			return;
		}

		const QUuid qt_property_uuid = Utilities::vadon_uuid_to_qt_uuid(property.id);
		if (type_it->properties.find(qt_property_uuid) != type_it->properties.end())
		{
			qWarning() << "Property already registered!";
			return;
		}

		PropertyData property_data;
		property_data.info = property;

		for (size_t type_list_index = 0; type_list_index < property.type_list_length; ++type_list_index)
		{
			property_data.type_list.push_back(Utilities::vadon_uuid_to_qt_uuid(type_list[type_list_index]));
		}

		type_it->properties.insert(qt_property_uuid, property_data);
		type_it->property_list.push_back(property.id);
	}

	::Vadon::Foundation::Property DataSchema::TypeMetadataRegistry::get_property_info(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_info", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_info", "Invalid property UUID!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_info", "Type not registered!");
			return ::Vadon::Foundation::Property{};
		}

		const QUuid qt_property_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
		auto property_it = type_it->properties.find(qt_property_uuid);
		if (property_it == type_it->properties.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_info", "Property not registered!");
			return ::Vadon::Foundation::Property{};
		}

		return property_it->info;
	}

	::Vadon::Foundation::UUID DataSchema::TypeMetadataRegistry::get_property_type_list_entry(const::Vadon::Foundation::UUID& type_uuid, const::Vadon::Foundation::UUID& property_uuid, size_t index) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_info", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_info", "Invalid property UUID!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_type_list_entry", "Type not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		const QUuid qt_property_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
		auto property_it = type_it->properties.find(qt_property_uuid);
		if (property_it == type_it->properties.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_type_list_entry", "Property not registered!");
			return ::Vadon::Foundation::UUID{};
		}

		const PropertyData& property_data = property_it.value();

		return Utilities::qt_uuid_to_vadon_uuid(property_data.type_list[index]);
	}

	void DataSchema::TypeMetadataRegistry::set_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key, const char* value)
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid property UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::set_property_metadata", "Invalid key!");
		
		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_property_metadata", "Type not registered!");
			return;
		}

		const QUuid qt_property_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
		auto property_it = type_it->properties.find(qt_property_uuid);

		if (property_it == type_it->properties.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::set_property_metadata", "Property not registered!");
			return;
		}

		property_it->metadata.insert(QString(key), QString(value).toUtf8());
	}

	const char* DataSchema::TypeMetadataRegistry::get_property_metadata(const ::Vadon::Foundation::UUID& type_uuid, const ::Vadon::Foundation::UUID& property_uuid, const char* key) const
	{
		Q_ASSERT_X(type_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid type UUID!");
		Q_ASSERT_X(property_uuid.is_valid() == true, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid property UUID!");
		Q_ASSERT_X(key != nullptr, "VadonEditor::Core::DataSchema::get_property_metadata", "Invalid key!");

		const QUuid qt_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
		auto type_it = m_types.find(qt_type_uuid);

		if (type_it == m_types.end())
		{
			// TODO: Q_ERROR macro?
			Q_ASSERT_X(false, "VadonEditor::Core::DataSchema::get_property_metadata", "Type not registered!");
			return nullptr;
		}

		const QUuid qt_property_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
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

	bool DataSchema::TypeMetadataRegistry::is_base_of(const QUuid& base_uuid, const QUuid& derived_uuid) const
	{
		Q_ASSERT_X(Utilities::is_uuid_valid(base_uuid) == true, "VadonEditor::Core::DataSchema::is_base_of", "Invalid type UUID!");
		Q_ASSERT_X(Utilities::is_uuid_valid(derived_uuid) == true, "VadonEditor::Core::DataSchema::is_base_of", "Invalid type UUID!");
		
		if (base_uuid == derived_uuid)
		{
			return true;
		}

		const ::Vadon::Foundation::UUID base_vadon_uuid = Utilities::qt_uuid_to_vadon_uuid(base_uuid);
		QUuid current_type_uuid = derived_uuid;

		while (Utilities::is_uuid_valid(current_type_uuid) == true)
		{
			auto type_it = m_types.find(current_type_uuid);
			Q_ASSERT_X(type_it != m_types.end(), "VadonEditor::Core::DataSchema::is_base_of", "Type not in schema!");

			const TypeData& type_data = type_it.value();
			if (type_data.info.base_id == base_vadon_uuid)
			{
				return true;
			}

			current_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_data.info.base_id);
		}

		return false;
	}

	void DataSchema::TypeMetadataRegistry::process_metadata()
	{
		for (auto type_it = m_types.begin(); type_it != m_types.end(); ++type_it)
		{
			TypeData& current_type_data = type_it.value();
			for (auto property_it = current_type_data.properties.begin(); property_it != current_type_data.properties.end(); ++property_it)
			{
				PropertyData& current_property_data = property_it.value();

				const QString flags_string = current_property_data.find_metadata(::Vadon::Foundation::CommonPropertyMetadata::Key::FLAGS);
				if (flags_string.isEmpty() == false)
				{
					const QStringList flag_string_list = flags_string.split(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
					for (const QString& current_flag_string : flag_string_list)
					{
						const ::Vadon::Foundation::CommonPropertyMetadata::Flags current_flag = ::Vadon::Foundation::CommonPropertyMetadata::parse_flag_string(current_flag_string.toLocal8Bit().constData());
						if (current_flag != ::Vadon::Foundation::CommonPropertyMetadata::Flags::NONE)
						{
							current_property_data.flags = static_cast<::Vadon::Foundation::CommonPropertyMetadata::Flags>(current_property_data.flags | current_flag);
						}
					}
				}
			}
		}

		// TODO: any other post-processing?
	}

	const TypeData* DataSchema::find_type_data(const QUuid& type_uuid) const
	{
		auto type_it = m_registry.m_types.find(type_uuid);
		if (type_it == m_registry.m_types.end())
		{
			// TODO: warning?
			return nullptr;
		}

		return &type_it.value();
	}

	const PropertyData* DataSchema::find_type_property_data(const QUuid& type_uuid, const QUuid& property_uuid) const
	{
		const TypeData* type_data = find_type_data(type_uuid);
		if (type_data == nullptr)
		{
			// TODO: warning?
			return nullptr;
		}

		while (type_data != nullptr)
		{
			const PropertyData* property_data = type_data->find_property_data(property_uuid);
			if (property_data != nullptr)
			{
				return property_data;
			}

			// If not found in this type, recursively try base type
			if (type_data->info.base_id.is_valid() == true)
			{
				type_data = find_type_data(Utilities::vadon_uuid_to_qt_uuid(type_data->info.base_id));
			}
			else
			{
				type_data = nullptr;
			}
		}

		return nullptr;
	}

	bool DataSchema::save_schema(const QString& schema_file_path)
	{
		QJsonObject data_schema_root;

		QJsonArray type_list_array;
		for (auto type_it = m_registry.m_types.begin(); type_it != m_registry.m_types.end(); ++type_it)
		{
			const TypeData& current_type_data = type_it.value();

			QJsonObject current_type_object;
			{
				QJsonObject type_info_object;

				const QUuid type_uuid = Utilities::vadon_uuid_to_qt_uuid(current_type_data.info.id);
				const QUuid base_uuid = Utilities::vadon_uuid_to_qt_uuid(current_type_data.info.base_id);

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

					QJsonArray type_list;
					for (const QUuid& current_type : current_property_data.type_list)
					{
						type_list.push_back(Utilities::uuid_to_base64_string(current_type));
					}
					property_object["type_list"] = type_list;

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

		// NOTE: loading into a temporary object, swapping with original if load is successful
		TypeMetadataRegistry loaded_metadata_registry;

		const QJsonObject& root_obj = schema_document.object();
		if (const QJsonValue type_arr_value = root_obj["type_list"]; type_arr_value.isArray())
		{
			loaded_metadata_registry.m_types.clear();

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
						new_type_data.info.id = Utilities::qt_uuid_to_vadon_uuid(new_type_uuid);
					}
					else
					{
						qCritical() << "Invalid schema format: cannot find type ID!";
						return false;
					}
					if (const QJsonValue base_uuid_value = type_info["base"]; base_uuid_value.isString())
					{
						new_type_data.info.base_id = Utilities::qt_uuid_to_vadon_uuid(json_string_to_uuid(base_uuid_value));
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
							new_property_data.info.id = Utilities::qt_uuid_to_vadon_uuid(property_uuid);
						}
						else
						{
							qCritical() << "Invalid schema format: cannot find property ID!";
							return false;
						}

						if (const QJsonValue property_type_list_value = current_property_obj["type_list"]; property_type_list_value.isArray())
						{
							const QJsonArray property_type_list_array = property_type_list_value.toArray();
							for (const QJsonValueConstRef& current_type_list_type : property_type_list_array)
							{
								new_property_data.type_list.push_back(json_string_to_uuid(current_type_list_type));
							}
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

				loaded_metadata_registry.m_types.insert(new_type_uuid, new_type_data);
				loaded_metadata_registry.m_type_list.push_back(new_type_data.info.id);
			}
		}
		else
		{
			qCritical() << "Invalid schema format: cannot find type list!";
			return false;
		}

		// Load successful
		loaded_metadata_registry.process_metadata();
		m_registry = loaded_metadata_registry;

		// Re-generate the Qt model
		generate_qt_model();
		return true;
	}

	QModelIndex DataSchema::find_type_index(const QUuid& type_uuid) const
	{
		const QStandardItem* found_item = find_type_tree_standard_item(m_qt_model.invisibleRootItem(), type_uuid);
		if (found_item != nullptr)
		{
			return found_item->index();
		}

		return QModelIndex();
	}

	::Vadon::Foundation::BaseType DataSchema::get_underlying_base_type(const QUuid& type_uuid) const
	{
		const ::Vadon::Foundation::BaseType base_type = TypeData::get_base_type(type_uuid);
		if (base_type != ::Vadon::Foundation::BaseType::INVALID)
		{
			return base_type;
		}

		if (is_base_of(Model::Resource::get_base_resource_type(), type_uuid) == true)
		{
			return ::Vadon::Foundation::BaseType::UUID;
		}

		return ::Vadon::Foundation::BaseType::INVALID;
	}

	void DataSchema::generate_qt_model()
	{
		// Clear all prior contents of the model
		m_qt_model.clear();

		QHash<QUuid, QStandardItem*> type_tree_item_lookup;

		QStandardItem* root_item = m_qt_model.invisibleRootItem();

		QStandardItem* base_types_root = create_type_tree_standard_item("Base Types");

		root_item->appendRow(base_types_root);

		const TypeMetadataRegistry& registry = get_registry();
		for (size_t type_index = 0; type_index < registry.get_registered_type_count(); ++type_index)
		{
			const ::Vadon::Foundation::UUID type_uuid = registry.get_type_uuid(type_index);
			const QUuid type_qt_uuid = VadonEditor::Utilities::vadon_uuid_to_qt_uuid(type_uuid);

			if (type_tree_item_lookup.find(type_qt_uuid) != type_tree_item_lookup.end())
			{
				// Node already added
				continue;
			}

			const VadonEditor::Core::TypeData* type_data = find_type_data(type_qt_uuid);

			QStandardItem* new_type_node = create_type_tree_standard_item(type_data->get_name());
			new_type_node->setData(type_qt_uuid, static_cast<int>(TypeTreeDataRole::TYPE_UUID));

			if (VadonEditor::Core::TypeData::get_base_type(Utilities::vadon_uuid_to_qt_uuid(type_uuid)) != ::Vadon::Foundation::BaseType::INVALID)
			{
				base_types_root->appendRow(new_type_node);
				continue;
			}

			type_tree_item_lookup.insert(type_qt_uuid, new_type_node);

			if (type_data->info.base_id.is_valid() == true)
			{
				// Has parent type, find or create node and add to it
				const QUuid base_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(type_data->info.base_id);
				auto tree_item_it = type_tree_item_lookup.find(base_qt_uuid);
				if (tree_item_it != type_tree_item_lookup.end())
				{
					tree_item_it.value()->appendRow(new_type_node);
				}
				else
				{
					// Parent type was not added yet, need to recursively add parent nodes
					QList<::Vadon::Foundation::UUID> parent_types;
					::Vadon::Foundation::UUID parent_id = type_data->info.base_id;
					while (true)
					{
						parent_types.push_back(parent_id);

						const VadonEditor::Core::TypeData* parent_type_data = find_type_data(Utilities::vadon_uuid_to_qt_uuid(parent_id));
						if (parent_type_data->info.base_id.is_valid() == false)
						{
							// No more ancestors, add to root
							break;
						}
						else if (type_tree_item_lookup.find(VadonEditor::Utilities::vadon_uuid_to_qt_uuid(parent_type_data->info.base_id)) != type_tree_item_lookup.end())
						{
							// Ancestor already added
							break;
						}
						else
						{
							// Ancestor also needs to be added
							parent_id = parent_type_data->info.base_id;
						}
					}

					// For each ancestor, add to root or parent
					while (parent_types.isEmpty() == false)
					{
						const ::Vadon::Foundation::UUID ancestor_id = parent_types.back();
						const VadonEditor::Core::TypeData* ancestor_type_data = find_type_data(Utilities::vadon_uuid_to_qt_uuid(ancestor_id));

						QStandardItem* ancestor_node = create_type_tree_standard_item(ancestor_type_data->get_name());

						const QUuid ancestor_qt_uuid = VadonEditor::Utilities::vadon_uuid_to_qt_uuid(ancestor_id);
						ancestor_node->setData(ancestor_qt_uuid, static_cast<int>(TypeTreeDataRole::TYPE_UUID));

						type_tree_item_lookup.insert(ancestor_qt_uuid, ancestor_node);

						if (ancestor_type_data->info.base_id.is_valid() == true)
						{
							auto ancestor_parent_it = type_tree_item_lookup.find(VadonEditor::Utilities::vadon_uuid_to_qt_uuid(ancestor_type_data->info.base_id));
							ancestor_parent_it.value()->appendRow(ancestor_node);
						}
						else
						{
							root_item->appendRow(ancestor_node);
						}

						parent_types.pop_back();
					}

					// Finally add the node itself
					tree_item_it = type_tree_item_lookup.find(base_qt_uuid);
					Q_ASSERT_X(tree_item_it != type_tree_item_lookup.end(), "VadonEditor::Core::DataSchema::generate_qt_model", "Cannot find parent!");

					tree_item_it.value()->appendRow(new_type_node);
				}
			}
			else
			{
				// No base type, add as root node
				root_item->appendRow(new_type_node);
			}
		}
	}

	TypeFilterModel::TypeFilterModel(const DataSchema& data_schema, QObject* parent)
		: QSortFilterProxyModel(parent)
		, m_data_schema(data_schema)
	{
	}

	void TypeFilterModel::set_root_type(const QUuid& type_id)
	{
		m_root_type = type_id;
		invalidateRowsFilter();
	}

	bool TypeFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
	{
		QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);

		// First check if any children passed filtering
		if (sourceModel()->hasChildren(source_index))
		{
			for (int child_index = 0; child_index < sourceModel()->rowCount(source_index); ++child_index)
			{
				if (filterAcceptsRow(child_index, source_index) == true)
				{
					// If child is accepted, the parent is accepted as well
					return true;
				}
			}
		}

		// Next check if we pass base filtering
		if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) == false)
		{
			return false;
		}

		// If we have a root type set, check if item is subclass
		if (Utilities::is_uuid_valid(m_root_type) == true)
		{
			const QUuid type_uuid = sourceModel()->data(source_index, static_cast<int>(TypeTreeDataRole::TYPE_UUID)).toUuid();
			if (Utilities::is_uuid_valid(type_uuid) == true)
			{
				return m_data_schema.is_base_of(m_root_type, type_uuid);
			}
			else
			{
				return false;
			}
		}

		return true;
	}
}