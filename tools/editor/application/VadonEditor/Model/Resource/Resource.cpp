#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/Model/Resource/File.hpp>

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
		if (m_pending_remove == true)
		{
			// Special case: Resource System is cleaning up, so we can early out
			return;
		}

		if (m_owner != nullptr)
		{
			// Remove from owner
			auto embedded_it = m_owner->m_embedded_resources.find(get_info().id);
			if (embedded_it != m_embedded_resources.end())
			{
				m_owner->m_embedded_resources.erase(embedded_it);
			}
			else
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Resource::~Resource", "Embedded resource not found in owner!");
			}
			m_owner = nullptr;
		}

		// Delete all embedded resources
		for (auto embedded_resource_it = m_embedded_resources.begin(); embedded_resource_it != m_embedded_resources.end(); ++embedded_resource_it)
		{
			Resource* current_embedded_resource = embedded_resource_it.value();
			current_embedded_resource->m_owner = nullptr; // Remove owner to skip callback

			delete current_embedded_resource;
		}

		m_embedded_resources.clear();

		m_pending_remove = true; // Set internal flag so ResourceSystem accepts the removal

		// Unregister from ResourceSystem
		m_application.get_model_system().get_resource_system().remove_resource(this);
	}

	void Resource::set_property(const PropertyID& property_id, const QVariant& value)
	{
		m_data.set_property(property_id, value);
		notify_modifed();
	}

	Resource* Resource::create_embedded_resource(const QUuid& type)
	{
		Resource* embedded_resource = m_application.get_model_system().get_resource_system().create_resource(type);
		if (embedded_resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Resource::create_embedded_resource", "Failed to create embedded resource!");
			return nullptr;
		}

		auto embedded_it = m_embedded_resources.find(embedded_resource->get_info().id);
		if (embedded_it != m_embedded_resources.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Resource::create_embedded_resource", "Failed to create embedded resource!");
			delete embedded_resource;
			return nullptr;
		}

		m_embedded_resources.insert(embedded_resource->get_info().id, embedded_resource);
		embedded_resource->m_owner = this;

		return embedded_resource;
	}

	bool Resource::is_resource_base_of_type(Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(get_base_resource_type(), type_id);
	}

	bool Resource::is_imported_file_base_of_type(Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid), type_id);
	}

	QUuid Resource::get_base_resource_type()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ResourceSchema::c_type_uuid);
	}

	QUuid Resource::get_imported_file_resource_type()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid);
	}

	Resource::Resource(Core::Application& application)
		: m_application(application)
		, m_owner(nullptr)
		, m_data(application)
		, m_pending_remove(false)
		, m_modified(false)
	{
	}

	bool Resource::initialize()
	{
		Q_ASSERT_X(m_info.id.isNull() == false, "VadonEditor::Model::Resource::initialize", "Invalid resource ID!");
		Q_ASSERT_X(m_info.type.isNull() == false, "VadonEditor::Model::Resource::initialize", "Invalid resource type!");

		if (m_data.initialize(m_info.type) == false)
		{
			return false;
		}

		for (auto embedded_resource_it = m_embedded_resources.begin(); embedded_resource_it != m_embedded_resources.end(); ++embedded_resource_it)
		{
			Resource* current_embedded_resource = embedded_resource_it.value();
			if (current_embedded_resource->initialize() == false)
			{
				return false;
			}
		}

		return true;
	}

	bool Resource::internal_save(QJsonObject& root_obj) const
	{
		// NOTE: cannot use the DataObject API to serialize entirely, as we need to place the ID and embedded
		// properties on the root
		// FIXME: find a way around this?
		{
			const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_id_property.id);
			root_obj[Utilities::serialize_labeled_uuid("id", id_property_uuid)] = Utilities::uuid_to_base64_string(m_info.id);

			const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_type_property.id);
			root_obj[Utilities::serialize_labeled_uuid("type", type_property_uuid)] = Utilities::uuid_to_base64_string(m_info.type);
		}

		{
			QJsonObject properties_object;
			if (m_data.serialize_properties(properties_object) == false)
			{
				return false;
			}

			const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_properties_property.id);
			root_obj[Utilities::serialize_labeled_uuid("properties", properties_property_uuid)] = properties_object;
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

		// NOTE: for the root keys, we have to do this suboptimal linear search
		// to be able to use labeled UUID keys
		const QUuid id_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_id_property.id);
		const QUuid type_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_type_property.id);
		const QUuid properties_property_uuid = Utilities::vadon_uuid_string_to_qt_uuid(Vadon::Foundation::ResourceSchema::c_properties_property.id);
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
					if (m_data.deserialize_properties(properties_value.toObject()) == false)
					{
						return false;
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

							embedded_resource_it = m_embedded_resources.insert(embedded_info.id, embedded_resource);
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