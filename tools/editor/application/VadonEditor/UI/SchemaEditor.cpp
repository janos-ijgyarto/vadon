#include <VadonEditor/UI/SchemaEditor.hpp>

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace
{
	QUuid json_string_to_uuid(const QJsonValue& uuid_string_value)
	{
		QString uuid_string = uuid_string_value.toString();
		QByteArray uuid_arr = QByteArray::fromBase64(uuid_string.toUtf8());
		return QUuid::fromBytes(uuid_arr);
	}
}

namespace VadonEditor::UI
{
	UI::TypeListModel::TypeListModel()
		: m_schema(nullptr)
	{
	}

	void TypeListModel::schema_loaded(const Schema* schema)
	{
		beginResetModel();
		m_schema = schema;
		endResetModel();
	}

	int TypeListModel::rowCount(const QModelIndex& /*parent*/) const
	{
		return m_schema != nullptr ? m_schema->types.size() : 0;
	}

	QVariant TypeListModel::data(const QModelIndex& index, int role) const
	{
		if (m_schema != nullptr)
		{
			if (role == Qt::DisplayRole)
			{
				const TypeData& type_data = m_schema->types[index.row()];
				return QString("ID: %1, Name: %2")
					.arg(type_data.id.toString(QUuid::StringFormat::WithoutBraces))
					.arg(type_data.name);
			}
		}

		return QVariant();
	}

	TypeInfoModel::TypeInfoModel(QObject* /*parent*/)
		: m_selected_type(nullptr)
	{
	}

	void TypeInfoModel::select_type(const TypeData* type_data)
	{
		beginResetModel();
		m_selected_type = type_data;
		endResetModel();
	}

	int TypeInfoModel::rowCount(const QModelIndex& /*parent*/) const
	{
		return m_selected_type != nullptr ? m_selected_type->properties.size() : 0;
	}

	int TypeInfoModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 3;
	}

	QVariant TypeInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
			switch (section) {
			case 0:
				return QString("ID");
			case 1:
				return QString("Type");
			case 2:
				return QString("Name");
			}
		}
		return QVariant();
	}

	QVariant TypeInfoModel::data(const QModelIndex& index, int role) const
	{
		if (m_selected_type != nullptr)
		{
			if (role == Qt::DisplayRole)
			{
				const PropertyData& property_data = m_selected_type->properties[index.row()];
				switch (index.column())
				{
				case 0:
					return property_data.id.toString(QUuid::StringFormat::WithoutBraces);
				case 1:
					return property_data.type.toString(QUuid::StringFormat::WithoutBraces);
				case 2:
					return property_data.name;
				}
			}
		}

		return QVariant();
	}

	SchemaEditor::SchemaEditor(QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		m_ui.metadataTableView->setModel(&m_type_info);
		m_ui.typeListView->setModel(&m_type_list);
	}

	void SchemaEditor::loadClicked()
	{
		QString schema_file = QFileDialog::getOpenFileName(this, "Select Schema File", QDir::currentPath(), tr("Schema Files (*.txt)"));
		if (schema_file.isEmpty() == false)
		{
			QFile load_file(schema_file);

			if (load_file.open(QIODevice::ReadOnly) == false)
			{
				qWarning("Failed to open schema file.");
				return;
			}

			// TODO: move this to its own object!
			QByteArray schema_data = load_file.readAll();

			// TODO: check error 
			QJsonDocument schema_document(QJsonDocument::fromJson(schema_data));
			if (schema_document.isNull() == true)
			{
				qWarning("Schema file is not valid JSON.");
				return;
			}

			const QJsonObject& root_obj = schema_document.object();
			if (const QJsonValue type_arr_value = root_obj["type_list"]; type_arr_value.isArray())
			{
				m_schema.types.clear();

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

					m_schema.types.push_back(new_type_data);
				}
			}

			m_type_info.select_type(nullptr);
			m_type_list.schema_loaded(&m_schema);
		}
	}

	void SchemaEditor::type_clicked(const QModelIndex& index)
	{
		const TypeData& type_data = m_schema.types[index.row()];
		m_ui.nameEdit->setText(type_data.name.isEmpty() ? "<UNNAMED>" : type_data.name);
		m_ui.uuidEdit->setText(type_data.id.toString(QUuid::StringFormat::WithoutBraces));

		m_type_info.select_type(&type_data);
	}
}