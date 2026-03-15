#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>

#include <QJsonArray>
#include <QJsonObject>

namespace VadonEditor::Model
{
	bool ResourceInfo::load(const QJsonObject& root_obj)
	{
		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_id_property.id);
		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_type_property.id);

		for (auto root_obj_it = root_obj.begin(); root_obj_it != root_obj.end(); ++root_obj_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(root_obj_it.key());
			if (entry_uuid == id_property_uuid)
			{
				if (const QJsonValueConstRef resource_id_value = root_obj_it.value(); resource_id_value.isString())
				{
					id = Utilities::base64_string_to_uuid(resource_id_value.toString());
				}
				else
				{
					qCritical() << "Invalid ID in resource data!";
					return false;
				}
			}
			else if (entry_uuid == type_property_uuid)
			{
				if (const QJsonValueConstRef resource_type_value = root_obj_it.value(); resource_type_value.isString())
				{
					type = Utilities::base64_string_to_uuid(resource_type_value.toString());
				}
				else
				{
					qCritical() << "Invalid type ID in resource data!";
					return false;
				}
			}
		}

		return true;
	}

	Resource::~Resource()
	{
		// Delete all embedded resources
		for (auto embedded_resource_it = m_embedded_resources.begin(); embedded_resource_it != m_embedded_resources.end(); ++embedded_resource_it)
		{
			delete embedded_resource_it.value();
		}

		m_embedded_resources.clear();
	}

	QVariant Resource::get_property(const PropertyID& property_id) const
	{
		auto property_it = m_properties.find(property_id);
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Model::Resource::get_property", "Cannot find property");

		return property_it.value();
	}

	void Resource::set_property(const PropertyID& property_id, const QVariant& value)
	{
		auto property_it = m_properties.find(property_id);
		Q_ASSERT_X(property_it != m_properties.end(), "VadonEditor::Model::Resource::set_property", "Cannot find property");		
		Q_ASSERT_X(property_it.value().typeId() == value.typeId(), "VadonEditor::Model::Resource::set_property", "Property value type mismatch");

		property_it.value() = value;
	}

	QVariant Resource::get_data(const QUuid& data_id) const
	{
		auto data_it = m_data.find(data_id);
		Q_ASSERT_X(data_it != m_data.end(), "VadonEditor::Model::Resource::get_data", "Cannot find data");

		return data_it.value();
	}

	void Resource::set_data(const QUuid& data_id, const QVariant& value)
	{
		auto data_it = m_data.find(data_id);
		Q_ASSERT_X(data_it != m_data.end(), "VadonEditor::Model::Resource::set_data", "Cannot find data");
		Q_ASSERT_X(data_it.value().typeId() == value.typeId(), "VadonEditor::Model::Resource::set_data", "Data value type mismatch");

		data_it.value() = value;
	}

	Resource::Resource(Core::Application& application)
		: m_application(application)
		, m_owner(nullptr)
	{
	}

	bool Resource::initialize()
	{
		Q_ASSERT_X(m_info.id.isNull() == false, "VadonEditor::Model::Resource::initialize", "Invalid resource ID!");
		Q_ASSERT_X(m_info.type.isNull() == false, "VadonEditor::Model::Resource::initialize", "Invalid resource type!");

		// Load default values into Resource
		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();

		::Vadon::Foundation::UUID current_type_uuid = Utilities::qt_uuid_to_vadon_uuid(m_info.type);
		while (current_type_uuid.is_valid() == true)
		{
			const Core::TypeData* type_data = data_schema.find_type_data(current_type_uuid);

			for (auto property_it = type_data->properties.begin(); property_it != type_data->properties.end(); ++property_it)
			{
				const ::Vadon::Foundation::BaseType base_type = Core::DataSchema::get_base_type(property_it->info.type);
				m_properties[property_it.key()] = Utilities::get_base_type_default_value(base_type);
			}

			current_type_uuid = type_data->info.base_id;
		}

		const QUuid resource_init_data = m_application.get_model_system().get_resource_system().get_resource_init_data(m_info.type);
		if (resource_init_data.isNull() == false)
		{
			m_data.insert(resource_init_data, QVariantMap());
		}

		return true;
	}

	bool Resource::internal_save(QJsonObject& root_obj) const
	{
		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();

		{
			const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_id_property.id);
			root_obj[Utilities::serialize_labeled_uuid("id", id_property_uuid)] = Utilities::uuid_to_base64_string(m_info.id);
			
			const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_type_property.id);
			root_obj[Utilities::serialize_labeled_uuid("type", type_property_uuid)] = Utilities::uuid_to_base64_string(m_info.type);
		}

		{
			QJsonObject properties_object;

			::Vadon::Foundation::UUID current_type_uuid = Utilities::qt_uuid_to_vadon_uuid(m_info.type);
			while (current_type_uuid.is_valid() == true)
			{
				const Core::TypeData* type_data = data_schema.find_type_data(current_type_uuid);

				for (auto property_it = type_data->properties.begin(); property_it != type_data->properties.end(); ++property_it)
				{
					const Core::PropertyData* type_property_data = type_data->find_property_data(Utilities::qt_uuid_to_vadon_uuid(property_it.key()));

					auto property_value_it = m_properties.find(property_it.key());
					Q_ASSERT_X(property_value_it != m_properties.end(), "VadonEditor::Model::Resource::internal_save", "Cannot find property value");

					const QString property_name = type_property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);

					QString key_string;
					if (property_name.isEmpty() == false)
					{
						key_string = Utilities::serialize_labeled_uuid(property_name, property_it.key());
					}
					else
					{
						key_string = Utilities::uuid_to_base64_string(property_it.key());
					}

					// NOTE: by this stage, the property contents should be something that can be directly serialized!
					properties_object[key_string] = QJsonValue::fromVariant(property_value_it.value());
				}

				current_type_uuid = type_data->info.base_id;
			}

			const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_properties_property.id);
			root_obj[Utilities::serialize_labeled_uuid("properties", properties_property_uuid)] = properties_object;
		}

		if(m_data.isEmpty() == false)
		{
			QJsonObject data_object;
			for (auto data_it = m_data.begin(); data_it != m_data.end(); ++data_it)
			{
				// FIXME: find a way to label the data entries?
				data_object[Utilities::uuid_to_base64_string(data_it.key())] = QJsonValue::fromVariant(data_it.value());
			}

			const QUuid data_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_data_property.id);
			root_obj[Utilities::serialize_labeled_uuid("data", data_property_uuid)] = data_object;
		}

		if (m_embedded_resources.isEmpty() == false)
		{
			QJsonArray embedded_array;
			for (auto embedded_it = m_embedded_resources.begin(); embedded_it != m_embedded_resources.end(); ++embedded_it)
			{
				QJsonObject current_embedded_obj;
				if (embedded_it.value()->internal_save(current_embedded_obj) == false)
				{
					return false;
				}
				embedded_array.push_back(current_embedded_obj);
			}

			const QUuid embedded_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_embedded_property.id);
			root_obj[Utilities::serialize_labeled_uuid("embedded_resources", embedded_property_uuid)] = embedded_array;
		}

		return true;
	}
	
	bool Resource::internal_load(const QJsonObject& root_obj)
	{
		// First validate info
		ResourceInfo info;
		if (info.load(root_obj) == false)
		{
			return false;
		}

		if (m_info.id != info.id)
		{
			qCritical() << "Invalid resource ID in resource data!";
			return false;
		}

		if (m_info.type != info.type)
		{
			qCritical() << "Invalid type ID in resource data!";
			return false;
		}

		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_id_property.id);
		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_type_property.id);
		const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_properties_property.id);
		const QUuid data_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_data_property.id);
		const QUuid embedded_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_embedded_property.id);
		for (auto root_obj_it = root_obj.begin(); root_obj_it != root_obj.end(); ++root_obj_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(root_obj_it.key());
			if (entry_uuid == id_property_uuid)
			{
				// TODO: validate ID?
			}
			else if (entry_uuid == type_property_uuid)
			{
				// TODO: validate type?
			}
			else if (entry_uuid == properties_property_uuid)
			{
				if (const QJsonValueConstRef properties_value = root_obj_it.value(); properties_value.isObject())
				{
					const QJsonObject properties_object = properties_value.toObject();
					for (auto property_it = properties_object.begin(); property_it != properties_object.end(); ++property_it)
					{
						const PropertyID current_property_id = Utilities::parse_labeled_uuid(property_it.key());
						auto property_data_it = m_properties.find(current_property_id);
						if (property_data_it != m_properties.end())
						{
							const QVariant property_value_variant = property_it.value().toVariant();
							if (property_value_variant.typeId() != property_data_it.value().typeId())
							{
								// NOTE: assume property data type changed, in which case it should have gotten a new UUID!
								qCritical() << "Mismatch in Resource data type!";
								return false;
							}
							property_data_it.value() = property_value_variant;
						}
						else
						{
							// NOTE: assume property was deprecated, ignore
							qWarning() << "Property not found in Resource!";
						}
					}
				}
			}
			else if (entry_uuid == data_property_uuid)
			{
				if (const QJsonValueConstRef data_value = root_obj_it.value(); data_value.isObject())
				{
					m_data.clear();

					const QJsonObject data_obj = data_value.toObject();
					for (auto data_it = data_obj.begin(); data_it != data_obj.end(); ++data_it)
					{
						m_data.insert(Utilities::parse_labeled_uuid(data_it.key()), data_it.value().toVariant());
					}
				}
			}
			else if (entry_uuid == embedded_property_uuid)
			{
				if (const QJsonValueConstRef embedded_value = root_obj_it.value(); embedded_value.isArray())
				{
					const QJsonArray embedded_array = embedded_value.toArray();
					for (auto current_embedded_resource : embedded_array)
					{
						if (current_embedded_resource.isObject() == false)
						{
							qCritical() << "Invalid embedded resource data!";
							return false;
						}
						const QJsonObject& embedded_resource_object = current_embedded_resource.toObject();
						ResourceInfo embedded_info;
						if (embedded_info.load(embedded_resource_object) == false)
						{
							return false;
						}

						auto embedded_resource_it = m_embedded_resources.find(embedded_info.id);
						if (embedded_resource_it == m_embedded_resources.end())
						{
							Resource* embedded_resource = new Resource(m_application);
							embedded_resource->m_info = embedded_info;

							if (embedded_resource->initialize() == false)
							{
								qCritical() << "Failed to create embedded resource!";
								delete embedded_resource;
								return false;
							}

							m_embedded_resources.insert(embedded_info.id, embedded_resource);
						}
						if (embedded_resource_it.value()->internal_load(embedded_resource_object) == false)
						{
							qCritical() << "Failed to load embedded resource data!";
							return false;
						}
					}
				}
			}
		}

		return true;
	}
}