#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Entity/EntityManager.hpp>

#include <Vadon/Model/Resource/ResourceSystem.hpp>
#include <Vadon/Model/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Data/Object.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>

namespace
{
	Vadon::ECS::EntityHandle find_entity_recursive(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle root_entity, const Vadon::Model::SceneID& scene_id, const ::Vadon::Foundation::UUID& entity_id)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto scene_component = component_manager.get_component<Vadon::Model::SceneComponent>(root_entity);
		VADON_ASSERT(scene_component.is_valid() == true, "Cannot find scene component!");

		// Make sure Entity is defined in this scene
		const Vadon::Model::EntitySceneInfo& entity_scene_info = scene_component->scene_info;
		if (entity_scene_info.scene_id != scene_id)
		{
			const Vadon::Model::EntitySceneInfo& parent_scene_info = scene_component->parent_scene_info;
			VADON_ASSERT(parent_scene_info.scene_id == scene_id, "Invalid Entity data in scene!");
			if (parent_scene_info.entity_id == entity_id)
			{
				return root_entity;
			}
			else
			{
				return Vadon::ECS::EntityHandle{};
			}
		}

		// Check ID for match
		if (entity_scene_info.entity_id == entity_id)
		{
			return root_entity;
		}

		// Entity is still defined in this scene, check children
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		const Vadon::ECS::EntityList current_entity_children = entity_manager.get_children(root_entity);
		for (Vadon::ECS::EntityHandle current_child_handle : current_entity_children)
		{
			Vadon::ECS::EntityHandle found_entity = find_entity_recursive(ecs_world, current_child_handle, scene_id, entity_id);
			if (found_entity.is_valid() == true)
			{
				return found_entity;
			}
		}

		return Vadon::ECS::EntityHandle();
	}

	void apply_sub_scene_entity_metadata(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity_handle, const Vadon::Model::SceneID& parent_scene_id, const Vadon::Model::SceneID& sub_scene_id)
	{
		auto scene_component = ecs_world.get_component_manager().get_component<Vadon::Model::SceneComponent>(entity_handle);
		if (scene_component->scene_info.scene_id != sub_scene_id)
		{
			return;
		}

		scene_component->parent_scene_info.scene_id = parent_scene_id;

		const Vadon::ECS::EntityList child_entities = ecs_world.get_entity_manager().get_children(entity_handle);
		for (Vadon::ECS::EntityHandle child_handle : child_entities)
		{
			apply_sub_scene_entity_metadata(ecs_world, child_handle, parent_scene_id, sub_scene_id);
		}
	}
}

namespace VadonEditor::Model
{
	Scene::~Scene()
	{
		VADON_ASSERT(is_loaded() == false, "Scene was not unloaded!");
	}

	Scene::Scene(Core::Editor& editor, Vadon::Model::SceneID id, Resource* resource)
		: m_editor(editor)
		, m_id(id)
		, m_resource(resource)
	{

	}

	bool Scene::initialize()
	{
		if (is_loaded() == true)
		{
			return true;
		}

		Vadon::Model::SceneSystem& engine_scene_system = m_editor.get_engine_core().get_system<Vadon::Model::SceneSystem>();
		const Vadon::Model::SceneHandle scene_handle = engine_scene_system.load_scene(m_id);

		m_root_entity = engine_scene_system.instantiate_scene(scene_handle, m_editor.get_ecs_world());
		VADON_ASSERT(m_root_entity.is_valid(), "Failed to instantiate scene");

		return true;
	}

	void Scene::close_scene()
	{
		if (is_loaded() == false)
		{
			return;
		}

		internal_remove_entity(m_root_entity);
	}

	void Scene::process_message(const char* data, size_t size)
	{
		::Vadon::Foundation::EditorMessageReader message_reader(data, size);
		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::MODEL:
		{
			const char* message_data = message_reader.get_current_message_data();
			const ::Vadon::Foundation::EditorModelMessageHeader* model_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageHeader*>(message_data);
			switch (model_message_header->message_type)
			{

			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_ADDED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityAdded* entity_added = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityAdded*>(message_data);
				internal_add_entity(entity_added->entity_info);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_MODIFIED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityModified* entity_modified = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityModified*>(message_data);
				process_entity_message(entity_modified->entity_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_REMOVED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityRemoved* entity_removed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityRemoved*>(message_data);
				process_entity_message(entity_removed->entity_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_ADDED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentAdded* component_added = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentAdded*>(message_data);
				process_entity_message(component_added->entity_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_PROPERTY_EDITED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited* component_property_edited = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited*>(message_data);
				process_entity_message(component_property_edited->entity_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_REMOVED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentRemoved* component_removed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentRemoved*>(message_data);
				process_entity_message(component_removed->entity_id, data, size);
			}
				break;
			}
		}
		break;
		}
	}

	void Scene::process_entity_message(const::Vadon::Foundation::UUID& entity_id, const char* data, size_t size)
	{
		const Vadon::ECS::EntityHandle entity_handle = internal_find_entity(entity_id);
		VADON_ASSERT(entity_handle.is_valid() == true, "Cannot find entity");

		::Vadon::Foundation::EditorMessageReader message_reader(data, size);
		switch (message_reader.get_current_category())
		{
		case ::Vadon::Foundation::EditorMessageCategory::MODEL:
		{
			const char* message_data = message_reader.get_current_message_data();
			const ::Vadon::Foundation::EditorModelMessageHeader* model_message_header = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageHeader*>(message_data);
			switch (model_message_header->message_type)
			{
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_MODIFIED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityModified* entity_modified = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityModified*>(message_data);

				const char* data_start = message_data + sizeof(::Vadon::Foundation::EditorModelMessageEntityModified);

				// TODO: propagate name to Scene data!
				std::string_view name_string_view(data_start, entity_modified->entity_name_length);
				Vadon::Core::Logger::log_message(std::format("Scene {} Entity {} name set to {}", Vadon::Utilities::uuid_to_string(m_id).string, Vadon::Utilities::uuid_to_string(entity_modified->entity_id).string, name_string_view));
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_REMOVED:
			{
				internal_remove_entity(entity_handle);
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_ADDED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentAdded* component_added = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentAdded*>(message_data);

				const Vadon::Utilities::TypeID component_type_id = Vadon::Utilities::TypeRegistry::get_type_id(component_added->component_id);
				VADON_ASSERT(component_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type ID!");

				Vadon::ECS::ComponentManager& component_manager = m_editor.get_ecs_world().get_component_manager();
				VADON_ASSERT(component_manager.has_component(entity_handle, component_type_id) == false, "Entity already has component!");

				component_manager.add_component(entity_handle, component_type_id);

				ComponentEvent component_event;
				component_event.type = ComponentEventType::ADDED;
				component_event.owner = entity_handle;
				component_event.component_type = component_type_id;

				m_editor.get_scene_system().dispatch_component_event(component_event);
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_PROPERTY_EDITED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited* component_property_edited = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited*>(message_data);

				const Vadon::Utilities::TypeID component_type_id = Vadon::Utilities::TypeRegistry::get_type_id(component_property_edited->component_id);
				VADON_ASSERT(component_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type ID!");

				Vadon::ECS::ComponentManager& component_manager = m_editor.get_ecs_world().get_component_manager();
				Vadon::ECS::ComponentHandle component_handle = component_manager.get_component(entity_handle, component_type_id);

				VADON_ASSERT(component_handle.is_valid() == true, "Entity does not have component");

				const void* data_start = message_data + sizeof(::Vadon::Foundation::EditorModelMessageComponentPropertyEdited);

				Vadon::Core::RawFileDataBuffer json_data_buffer;
				json_data_buffer.resize(component_property_edited->data_size);

				memcpy(std::to_address(json_data_buffer.begin()), data_start, component_property_edited->data_size);

				Vadon::Utilities::Serializer::Instance serializer_instance = Vadon::Utilities::Serializer::create_serializer(json_data_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);
				if (serializer_instance->initialize() == false)
				{
					VADON_ERROR("Failed to deserialize message data!");
					return;
				}

				Vadon::Utilities::ObjectWrapper component_wrapper(component_type_id, component_handle.get_raw());
				if (load_component_property_data(*serializer_instance, component_wrapper, entity_handle) == false)
				{
					VADON_ERROR("Failed to load property data!");
					return;
				}

				if (serializer_instance->finalize() == false)
				{
					VADON_ERROR("Failed to finalize message serializer!");
					return;
				}

				ComponentEvent component_event;
				component_event.type = ComponentEventType::EDITED;
				component_event.owner = entity_handle;
				component_event.component_type = component_type_id;

				m_editor.get_scene_system().dispatch_component_event(component_event);
			}
			break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_REMOVED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentRemoved* component_removed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentRemoved*>(message_data);

				const Vadon::Utilities::TypeID component_type_id = Vadon::Utilities::TypeRegistry::get_type_id(component_removed->component_id);
				VADON_ASSERT(component_type_id != Vadon::Utilities::TypeID::INVALID, "Invalid type ID!");

				Vadon::ECS::ComponentManager& component_manager = m_editor.get_ecs_world().get_component_manager();
				VADON_ASSERT(component_manager.has_component(entity_handle, component_type_id) == true, "Entity does not have component!");

				ComponentEvent component_event;
				component_event.type = ComponentEventType::REMOVED;
				component_event.owner = entity_handle;
				component_event.component_type = component_type_id;

				m_editor.get_scene_system().dispatch_component_event(component_event);

				component_manager.remove_component(entity_handle, component_type_id);
			}
			break;
			}
		}
		}
	}

	Vadon::ECS::EntityHandle Scene::internal_find_entity(const ::Vadon::Foundation::UUID& entity_id) const
	{
		Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();
		return find_entity_recursive(ecs_world, m_root_entity, m_id, entity_id);
	}

	void Scene::internal_add_entity(const::Vadon::Foundation::SceneEntityInfo& entity_info)
	{
		VADON_ASSERT(internal_find_entity(entity_info.id).is_valid() == false, "Entity already added with this ID!");

		Vadon::ECS::EntityHandle parent_entity;
		if (entity_info.parent.is_valid() == true)
		{
			parent_entity = internal_find_entity(entity_info.parent);
			VADON_ASSERT(parent_entity.is_valid() == true, "Cannot find parent entity!");
		}
		else
		{
			// If we try to replace the root entity, it must be removed first
			VADON_ASSERT(m_root_entity.is_valid() == false, "Root entity still valid!");
		}

		Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		Vadon::ECS::EntityHandle new_entity;
		if (entity_info.sub_scene.is_valid() == true)
		{
			// Instantiate sub-scene
			const Vadon::Model::SceneID sub_scene_id = Vadon::Model::SceneID::from_resource_id(entity_info.sub_scene);

			Vadon::Model::SceneSystem& engine_scene_system = m_editor.get_engine_core().get_system<Vadon::Model::SceneSystem>();
			const Vadon::Model::SceneHandle sub_scene_handle = engine_scene_system.load_scene(sub_scene_id);
			VADON_ASSERT(sub_scene_handle.is_valid() == true, "Failed to load sub-scene");

			new_entity = engine_scene_system.instantiate_scene(sub_scene_handle, ecs_world);

			// For sub-scenes, we also have to add some metadata
			// This is only needed here, since we are adding this sub-scene at design time
			apply_sub_scene_entity_metadata(ecs_world, new_entity, m_id, sub_scene_id);

			auto root_scene_component = component_manager.get_component<Vadon::Model::SceneComponent>(new_entity);
			root_scene_component->parent_scene_info.entity_id = entity_info.id;
		}
		else
		{
			// Add a new empty Entity
			new_entity = entity_manager.create_entity();

			auto scene_component = component_manager.add_component<Vadon::Model::SceneComponent>(new_entity);
			scene_component->scene_info.scene_id = m_id;
			scene_component->scene_info.entity_id = entity_info.id;
		}

		if (parent_entity.is_valid() == true)
		{
			// Add to parent
			entity_manager.add_child_entity(parent_entity, new_entity);
		}
		else
		{
			m_root_entity = new_entity;
		}

		entity_added(new_entity);
	}

	void Scene::internal_remove_entity(Vadon::ECS::EntityHandle entity_handle)
	{
		// TODO: double-check that this Entity belongs to this scene?
		entity_removed(entity_handle);
		m_editor.get_ecs_world().remove_entity(entity_handle);

		if (m_root_entity == entity_handle)
		{
			m_root_entity.invalidate();
		}
	}

	bool Scene::load_component_property_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::ObjectWrapper& component_wrapper, Vadon::ECS::EntityHandle owner_entity)
	{
		Vadon::Utilities::VariantDictionary component_properties;
		if (Vadon::Utilities::ObjectSerializer::serialize_object_properties(serializer, component_wrapper.get_type(), component_properties) == false)
		{
			VADON_ERROR("Failed to deserialize component properties!");
			return false;
		}

		if (Vadon::Utilities::ObjectSerializer::load_object_property_data(component_wrapper, component_properties) == false)
		{
			VADON_ERROR("Failed to deserialize component properties!");
			return false;
		}

		// FIXME: use iterator instead of requesting keys!
		SceneSystem& scene_system = m_editor.get_scene_system();
		const Vadon::Utilities::Serializer::KeyVector property_keys = serializer.get_keys();
		for (const std::string& current_key : property_keys)
		{
			Vadon::Foundation::UUID current_property_id;
			if (Vadon::Utilities::uuid_from_base64_string(current_key, current_property_id) == false)
			{
				VADON_ERROR("Invalid data in component!");
				return false;
			}

			ComponentEvent component_event;
			component_event.type = ComponentEventType::EDITED;
			component_event.owner = owner_entity;
			component_event.component_type = component_wrapper.get_type();

			scene_system.dispatch_component_event(component_event);

			// TODO: only log this in debug?
			Vadon::Core::Logger::log_message(std::format("Modified component {} property {}\n", Vadon::Utilities::uuid_to_string(get_id()).string, Vadon::Utilities::uuid_to_string(current_property_id).string));
		}

		return true;
	}

	void Scene::entity_added(Vadon::ECS::EntityHandle entity_handle)
	{
		m_editor.get_scene_system().dispatch_entity_event(EntityEvent{ .entity = entity_handle, .type = EntityEventType::ADDED });

		Vadon::ECS::EntityManager& entity_manager = m_editor.get_ecs_world().get_entity_manager();
		for (Vadon::ECS::EntityHandle child_entity : entity_manager.get_children(entity_handle))
		{
			entity_added(child_entity);
		}
	}

	void Scene::entity_removed(Vadon::ECS::EntityHandle entity_handle)
	{
		m_editor.get_scene_system().dispatch_entity_event(EntityEvent{ .entity = entity_handle, .type = EntityEventType::REMOVED });

		Vadon::ECS::EntityManager& entity_manager = m_editor.get_ecs_world().get_entity_manager();
		for (Vadon::ECS::EntityHandle child_entity : entity_manager.get_children(entity_handle))
		{
			entity_removed(child_entity);
		}
	}
}