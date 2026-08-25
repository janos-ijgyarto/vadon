#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Network/NetworkSystem.hpp>
#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>

#include <QJsonDocument>
#include <QJsonObject>

namespace VadonEditor::Model
{
	Scene::~Scene()
	{
		if (m_resource == nullptr)
		{
			// Special case: Scene System is cleaning up, so we can early out
			return;
		}

		Resource* resource = m_resource;
		m_resource = nullptr;

		// Unregister from Scene system
		m_application.get_model_system().get_scene_system().remove_scene(this);

		// Delete the Resource
		// TODO: decrement refcount instead!
		delete resource;
	}
	
	bool Scene::is_modified() const
	{
		return m_resource->is_modified();
	}

	void Scene::notify_modified()
	{
		m_resource->notify_modifed();
	}

	bool Scene::is_scene_base_of_type(Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(get_scene_type_uuid(), type_id);
	}

	QUuid Scene::get_scene_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid);
	}

	QUuid Scene::get_scene_entities_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_entities_property.id);
	}

	void Scene::open_scene()
	{
		message_scene_opened(false);
	}

	bool Scene::save_scene() const
	{
		if (store_scene_data() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Scene::save_scene", "Failed to save scene data");
			return false;
		}

		return m_application.get_model_system().get_resource_system().save_resource(m_resource);
	}

	bool Scene::load_scene()
	{
		const QUuid scene_entities_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_entities_property.id);
		QVariant entity_list_variant = m_resource->get_property(scene_entities_uuid);

		if (m_entity_model.load_data(entity_list_variant.toList()) == false)
		{
			return false;
		}

		message_scene_opened(true);

		return true;
	}

	QUuid Scene::instantiate_scene(const QUuid& scene_id, Entity* parent)
	{
		if (is_sub_scene_acyclic(scene_id) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Scene::instantiate_scene", "Instantiated scene would create infinite recursion");
			return QUuid();
		}

		return m_entity_model.instantiate_scene(scene_id, parent);
	}

	Scene::Scene(Core::Application& application, Resource* resource)
		: m_application(application)
		, m_id(resource->get_info().id)
		, m_resource(resource)
		, m_entity_model(application)
	{

	}

	bool Scene::initialize()
	{
		QObject::connect(&m_entity_model, &EntityModel::entity_added,
			[this](const QUuid& id)
			{
				entity_added(id);
			}
		);
		QObject::connect(&m_entity_model, &EntityModel::entity_removed,
			[this](const QUuid& id)
			{
				entity_removed(id);
			}
		);
		QObject::connect(&m_entity_model, &EntityModel::entity_name_changed, 
			[this](const QUuid& id)
			{
				entity_name_changed(id);
			}
		);

		QObject::connect(&m_entity_model, &EntityModel::entity_component_added,
			[this](const QUuid& entity_id, const QUuid& component_id)
			{
				entity_component_added(entity_id, component_id);
			}
		);
		QObject::connect(&m_entity_model, &EntityModel::entity_component_removed,
			[this](const QUuid& entity_id, const QUuid& component_id)
			{
				entity_component_removed(entity_id, component_id);
			}
		);
		QObject::connect(&m_entity_model, &EntityModel::entity_component_property_edited,
			[this](const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id)
			{
				entity_component_property_edited(entity_id, component_id, property_id);
			}
		);

		return true;
	}

	bool Scene::store_scene_data() const
	{
		QVariantList entity_list;
		if (m_entity_model.save_data(entity_list) == false)
		{
			return false;
		}
		
		const QUuid scene_entities_uuid = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_entities_property.id);
		m_resource->set_property(scene_entities_uuid, entity_list);
		return true;
	}

	void Scene::entity_added(const QUuid& id)
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageEntityAdded entity_added_message;
		entity_added_message.message_type = ::Vadon::Foundation::EditorModelMessageType::ENTITY_ADDED;

		entity_added_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);
		entity_added_message.entity_info.id = Utilities::qt_uuid_to_vadon_uuid(id);

		Entity* entity = m_entity_model.find_entity_by_id(id);
		Q_ASSERT_X(entity != nullptr, "VadonEditor::Model::Scene::entity_added", "Cannot find entity");

		entity_added_message.entity_info.parent = Utilities::qt_uuid_to_vadon_uuid(entity->get_parent());
		entity_added_message.entity_info.sub_scene = Utilities::qt_uuid_to_vadon_uuid(entity->get_sub_scene_id());

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, entity_added_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Scene::entity_removed(const QUuid& id)
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageEntityRemoved entity_removed_message;
		entity_removed_message.message_type = ::Vadon::Foundation::EditorModelMessageType::ENTITY_REMOVED;

		entity_removed_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);
		entity_removed_message.entity_id = Utilities::qt_uuid_to_vadon_uuid(id);

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, entity_removed_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Scene::entity_name_changed(const QUuid& id)
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageEntityModified entity_modified_message;
		entity_modified_message.message_type = ::Vadon::Foundation::EditorModelMessageType::ENTITY_MODIFIED;

		entity_modified_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);

		entity_modified_message.entity_id = Utilities::qt_uuid_to_vadon_uuid(id);

		Entity* entity = m_entity_model.find_entity_by_id(id);
		const QString entity_name = entity->get_name();

		entity_modified_message.entity_name_length = entity_name.length();

		char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::MODEL, sizeof(::Vadon::Foundation::EditorModelMessageEntityModified) + entity_modified_message.entity_name_length);
		memcpy(message_data, &entity_modified_message, sizeof(::Vadon::Foundation::EditorModelMessageEntityModified));
		memcpy(message_data + sizeof(::Vadon::Foundation::EditorModelMessageEntityModified), entity_name.toUtf8().constData(), entity_modified_message.entity_name_length);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Scene::entity_component_added(const QUuid& entity_id, const QUuid& component_id)
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageComponentAdded component_added_message;
		component_added_message.message_type = ::Vadon::Foundation::EditorModelMessageType::COMPONENT_ADDED;

		component_added_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);
		component_added_message.entity_id = Utilities::qt_uuid_to_vadon_uuid(entity_id);
		component_added_message.component_id = Utilities::qt_uuid_to_vadon_uuid(component_id);

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, component_added_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Scene::entity_component_removed(const QUuid& entity_id, const QUuid& component_id)
	{
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageComponentRemoved component_removed_message;
		component_removed_message.message_type = ::Vadon::Foundation::EditorModelMessageType::COMPONENT_REMOVED;

		component_removed_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);
		component_removed_message.entity_id = Utilities::qt_uuid_to_vadon_uuid(entity_id);
		component_removed_message.component_id = Utilities::qt_uuid_to_vadon_uuid(component_id);

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, component_removed_message);

		m_application.get_network_system().send_message(message_serializer);
	}

	void Scene::entity_component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id)
	{
		// FIXME: should we only send a message if a plugin is attached?
		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageComponentPropertyEdited component_property_edited;
		component_property_edited.message_type = ::Vadon::Foundation::EditorModelMessageType::COMPONENT_PROPERTY_EDITED;

		component_property_edited.scene_id = Utilities::qt_uuid_to_vadon_uuid(m_id);
		component_property_edited.entity_id = Utilities::qt_uuid_to_vadon_uuid(entity_id);
		component_property_edited.component_id = Utilities::qt_uuid_to_vadon_uuid(component_id);
		component_property_edited.property_id = Utilities::qt_uuid_to_vadon_uuid(property_id);

		const Entity* entity = m_entity_model.find_entity_by_id(entity_id);
		const Component* component = entity->get_component(component_id);

		// NOTE: skip label, we are only serializing to message the plugin
		QJsonObject property_object;
		if (component->serialize_property(property_id, property_object, false) == false)
		{
			Q_ASSERT_X(false, "Scene::entity_component_property_edited", "Failed to serialize property");
			return;
		}

		QJsonDocument property_obj_document(property_object);
		QByteArray json_data = property_obj_document.toJson(QJsonDocument::JsonFormat::Compact); // Serialize as compact JSON to minimize overhead

		component_property_edited.data_size = json_data.size();

		char* message_data = message_serializer.allocate_message(::Vadon::Foundation::EditorMessageCategory::MODEL, sizeof(::Vadon::Foundation::EditorModelMessageComponentPropertyEdited) + json_data.size());

		memcpy(message_data, &component_property_edited, sizeof(::Vadon::Foundation::EditorModelMessageComponentPropertyEdited));
		memcpy(message_data + sizeof(::Vadon::Foundation::EditorModelMessageComponentPropertyEdited), json_data.constData(), json_data.size());

		m_application.get_network_system().send_message(message_serializer);
	}

	bool Scene::is_sub_scene_acyclic(const QUuid& scene_id)
	{
		if (m_id == scene_id)
		{
			return false;
		}

		const Scene* instantiated_scene = m_application.get_model_system().get_scene_system().get_scene(scene_id);
		const EntityModel& entity_model = instantiated_scene->get_entity_model();

		const QList<QUuid> entity_id_list = entity_model.get_entity_id_list();
		for (const QUuid& current_entity_id : entity_id_list)
		{
			const Entity* current_entity = entity_model.find_entity_by_id(current_entity_id);
			const SceneID sub_scene_id = current_entity->get_sub_scene_id();
			if (Utilities::is_uuid_valid(sub_scene_id) == true)
			{
				// Entity is an instantiated scene, perform recursive check
				if (is_sub_scene_acyclic(sub_scene_id) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	void Scene::message_scene_opened(bool reload)
	{
		// TODO: use the "reload" flag to indicate whether the client should reload scene, or if they should just ensure it's loaded?
		Q_UNUSED(reload);

		// FIXME: use temp allocator or shared serializer
		VadonEditor::Network::MessageSerializer message_serializer;

		::Vadon::Foundation::EditorModelMessageSceneOpened scene_opened_message;
		scene_opened_message.message_type = ::Vadon::Foundation::EditorModelMessageType::SCENE_OPENED;

		scene_opened_message.scene_id = Utilities::qt_uuid_to_vadon_uuid(get_id());

		message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, scene_opened_message);

		m_application.get_network_system().send_message(message_serializer);
	}
}