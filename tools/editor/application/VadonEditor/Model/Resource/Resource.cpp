#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/Utilities/UUID.hpp>
#include <VadonEditor/Utilities/Data/Variant.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>
#include <Vadon/Foundation/Model/Resource/File.hpp>

#include <QJsonArray>
#include <QJsonDocument>
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

	void Resource::notify_modifed()
	{
		m_modified = true;
		update_asset_state();
		if (m_owner != nullptr)
		{
			m_owner->notify_modifed();
		}
	}

	void Resource::set_property(const PropertyID& property_id, const QVariant& value)
	{
		m_data.set_property(property_id, value);
		notify_modifed();

		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageResourcePropertyEdited property_edited_message;
		property_edited_message.message_type = ::Vadon::Foundation::EditorModelMessageType::RESOURCE_PROPERTY_EDITED;

		property_edited_message.resource_id = Utilities::qt_uuid_to_vadon_uuid(m_info.id);
		property_edited_message.property_id = Utilities::qt_uuid_to_vadon_uuid(property_id);

		QJsonObject property_object;
		if (m_data.serialize_property_data(property_id, property_object) == false)
		{
			Q_ASSERT_X(false, "Resource::set_property", "Failed to serialize property");
			return;
		}

		QJsonDocument property_obj_document(property_object);
		QByteArray json_data = property_obj_document.toJson(QJsonDocument::JsonFormat::Compact); // Serialize as compact JSON to minimize overhead

		property_edited_message.data_size = json_data.size();

		char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::MODEL, sizeof(::Vadon::Foundation::EditorModelMessageResourcePropertyEdited) + json_data.size());

		memcpy(message_data, &property_edited_message, sizeof(::Vadon::Foundation::EditorModelMessageResourcePropertyEdited));
		memcpy(message_data + sizeof(::Vadon::Foundation::EditorModelMessageResourcePropertyEdited), json_data.constData(), json_data.size());

		m_application.get_network_system().send_message(message_serializer);
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

		{
			// FIXME: use temp allocator or shared serializer
			VadonEditor::Network::MessageSerializer message_serializer;

			::Vadon::Foundation::EditorModelMessageResourceAddEmbedded add_embedded_message;
			add_embedded_message.message_type = ::Vadon::Foundation::EditorModelMessageType::RESOURCE_ADD_EMBEDDED;

			add_embedded_message.resource_id = Utilities::qt_uuid_to_vadon_uuid(m_info.id);
			add_embedded_message.embedded_id = Utilities::qt_uuid_to_vadon_uuid(embedded_resource->get_info().id);

			message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, add_embedded_message);

			m_application.get_network_system().send_message(message_serializer);
		}

		return embedded_resource;
	}

	void Resource::open()
	{
		message_resource_loaded(false);
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

		if (m_data.default_initialize(m_info.type) == false)
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
							embedded_resource->m_owner = this;

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

		message_resource_loaded(true);

		return true;
	}

	void Resource::message_resource_created() const
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageResourceCreated resource_created_message;
		resource_created_message.message_type = ::Vadon::Foundation::EditorModelMessageType::RESOURCE_CREATED;

		resource_created_message.type_id = Utilities::qt_uuid_to_vadon_uuid(m_info.type);
		resource_created_message.resource_id = Utilities::qt_uuid_to_vadon_uuid(m_info.id);

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, resource_created_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Resource::message_resource_asset_created() const
	{
		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		const int asset_id = m_application.get_model_system().get_resource_system().find_resource_asset_id(m_info.id);
		Q_ASSERT_X(asset_id != Core::AssetInfo::c_invalid_file_id, "VadonEditor::Model::Resource::message_resource_asset_created", "Cannot find resource asset");

		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		Q_ASSERT_X(asset_index.isValid(), "VadonEditor::Model::Resource::message_resource_asset_created", "Cannot find asset index");

		const Core::AssetInfo asset_info = asset_manager.get_asset_info(asset_index);

		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageResourceAssetCreated resource_asset_created_message;
		resource_asset_created_message.message_type = ::Vadon::Foundation::EditorModelMessageType::RESOURCE_ASSET_CREATED;

		resource_asset_created_message.resource_id = Utilities::qt_uuid_to_vadon_uuid(m_info.id);
		resource_asset_created_message.asset_path_length = asset_info.path.length();

		// FIXME: create a more elegant way to add a string to a message
		char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::MODEL, sizeof(::Vadon::Foundation::EditorModelMessageResourceAssetCreated) + resource_asset_created_message.asset_path_length);
		memcpy(message_data, &resource_asset_created_message, sizeof(::Vadon::Foundation::EditorModelMessageResourceAssetCreated));
		memcpy(message_data + sizeof(::Vadon::Foundation::EditorModelMessageResourceAssetCreated), asset_info.path.toUtf8().constData(), resource_asset_created_message.asset_path_length);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Resource::message_resource_loaded(bool reload) const
	{
		// Send message about Resource
		if (is_embedded() == true)
		{
			// Only if not embedded, the embedded resources will be loaded by the simulator independently
			return;
		}

		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageResourceLoaded resource_loaded_message;
		resource_loaded_message.message_type = ::Vadon::Foundation::EditorModelMessageType::RESOURCE_LOADED;

		resource_loaded_message.resource_id = Utilities::qt_uuid_to_vadon_uuid(m_info.id);
		resource_loaded_message.reload = reload;

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, resource_loaded_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Resource::clear_modified()
	{
		m_modified = false;
		update_asset_state();
	}

	void Resource::update_asset_state()
	{
		if (is_embedded() == true)
		{
			return;
		}

		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		const int asset_id = m_application.get_model_system().get_resource_system().find_resource_asset_id(m_info.id);
		if (asset_id == Core::AssetInfo::c_invalid_file_id)
		{
			// Resource may not have an asset yet (e.g new scene)
			return;
		}

		const QModelIndex asset_index = asset_manager.find_asset_index(asset_id);
		Q_ASSERT_X(asset_index.isValid(), "VadonEditor::Model::Resource::update_asset_state", "Cannot find asset index");

		asset_manager.set_asset_modified(asset_index, m_modified);
	}
}