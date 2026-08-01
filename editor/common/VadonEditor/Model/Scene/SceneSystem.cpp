#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Core/Logger.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/ComponentManager.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>

namespace VadonEditor::Model
{
	SceneSystem::~SceneSystem()
	{
		VADON_ASSERT(m_scene_lookup.empty() == true, "Did not clear scenes!");
	}

	const Scene* SceneSystem::find_scene(const Vadon::Model::SceneID& scene_id) const
	{
		auto scene_it = m_scene_lookup.find(scene_id);
		if (scene_it != m_scene_lookup.end())
		{
			return scene_it->second;
		}

		return nullptr;
	}

	Scene* SceneSystem::get_scene(const Vadon::Model::SceneID& scene_id)
	{
		Scene* scene = find_scene(scene_id);
		if (scene != nullptr)
		{
			return scene;
		}

		// Make sure resource exists
		Resource* scene_resource = m_editor.get_resource_system().find_resource(scene_id);
		VADON_ASSERT(scene_resource != nullptr, "Scene resource not found!");

		scene = new Scene(m_editor, scene_id, scene_resource);
		if (scene->initialize() == false)
		{
			VADON_ERROR("Failed to open scene!");
			delete scene;
			return nullptr;
		}

		// Add scene to lookup
		m_scene_lookup.insert(std::make_pair(scene_id, scene));

		// Dispatch event for Scene root entity
		// Have to do this AFTER the scene has been added to the lookup
		// FIXME: this makes things a bit complicated, would be nicer to do it async?
		scene->entity_added(scene->m_root_entity);

		return scene;
	}

	void SceneSystem::add_entity_event_callback(EntityEventCallback callback)
	{
		m_entity_callbacks.push_back(callback);
	}

	void SceneSystem::add_component_event_callback(ComponentEventCallback callback)
	{
		m_component_callbacks.push_back(callback);
	}

	void SceneSystem::dispatch_entity_event(const EntityEvent& event)
	{
		for (const EntityEventCallback& callback : m_entity_callbacks)
		{
			callback(event);
		}
	}

	void SceneSystem::dispatch_component_event(const ComponentEvent& event)
	{
		for (const ComponentEventCallback& callback : m_component_callbacks)
		{
			callback(event);
		}
	}

	const Scene* SceneSystem::find_entity_scene(Vadon::ECS::EntityHandle entity) const
	{
		Vadon::ECS::World& ecs_world = m_editor.get_ecs_world();
		const Vadon::ECS::EntityHandle root_entity = ecs_world.get_entity_manager().get_entity_root(entity);

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto root_scene_component = component_manager.get_component<Vadon::Model::SceneComponent>(entity);

		VADON_ASSERT(root_scene_component.is_valid() == true, "Cannot find Scene component");

		const Scene* root_scene = find_scene(root_scene_component->scene_info.scene_id);
		VADON_ASSERT(root_scene != nullptr, "Cannot find root Scene");

		return root_scene;
	}

	SceneSystem::SceneSystem(Core::Editor& editor)
		: m_editor(editor)
		, m_selected_scene(nullptr)
	{

	}

	bool SceneSystem::initialize()
	{
		return true;
	}

	void SceneSystem::shutdown()
	{
		for (const auto scene_pair : m_scene_lookup)
		{
			Scene* current_scene = scene_pair.second;
			current_scene->close_scene();
			delete current_scene;
		}

		m_scene_lookup.clear();
	}

	void SceneSystem::process_message(const char* data, size_t size)
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
			case ::Vadon::Foundation::EditorModelMessageType::SCENE_OPENED:
			{
				const ::Vadon::Foundation::EditorModelMessageSceneOpened* scene_opened_message = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageSceneOpened*>(message_data);

				const Vadon::Model::SceneID scene_id = Vadon::Model::SceneID::from_resource_id(scene_opened_message->scene_id);
				Scene* scene = find_scene(scene_id);
				if (scene != nullptr)
				{
					// Remove resource, since we will reload
					internal_remove_scene(scene);
				}

				scene = get_scene(scene_id);
				VADON_ASSERT(scene != nullptr, "Failed to create scene!");

				Vadon::Core::Logger::log_message(std::format("Opened scene {}\n", Vadon::Utilities::uuid_to_string(scene_id).string));
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::SCENE_SELECTED:
			{
				const ::Vadon::Foundation::EditorModelMessageSceneSelected* scene_selected_message = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageSceneSelected*>(message_data);

				if (scene_selected_message->scene_id.is_valid() == true)
				{
					const Vadon::Model::SceneID scene_id = Vadon::Model::SceneID::from_resource_id(scene_selected_message->scene_id);
					Scene* scene = find_scene(scene_id);
					VADON_ASSERT(scene != nullptr, "Scene not loaded!");

					m_selected_scene = scene;
				}
				else
				{
					m_selected_scene = nullptr;
				}
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::SCENE_CLOSED:
			{
				const ::Vadon::Foundation::EditorModelMessageSceneClosed* scene_closed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageSceneClosed*>(message_data);

				const Vadon::Model::SceneID scene_id = Vadon::Model::SceneID::from_resource_id(scene_closed->scene_id);
				Scene* scene = find_scene(scene_id);
				VADON_ASSERT(scene != nullptr, "Scene not loaded!");

				internal_remove_scene(scene);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_ADDED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityAdded* entity_added = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityAdded*>(message_data);
				process_scene_message(entity_added->scene_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_MODIFIED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityModified* entity_modified = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityModified*>(message_data);
				process_scene_message(entity_modified->scene_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::ENTITY_REMOVED:
			{
				const ::Vadon::Foundation::EditorModelMessageEntityRemoved* entity_removed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageEntityRemoved*>(message_data);
				process_scene_message(entity_removed->scene_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_ADDED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentAdded* component_added = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentAdded*>(message_data);
				process_scene_message(component_added->scene_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_PROPERTY_EDITED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited* component_property_edited = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentPropertyEdited*>(message_data);
				process_scene_message(component_property_edited->scene_id, data, size);
			}
				break;
			case ::Vadon::Foundation::EditorModelMessageType::COMPONENT_REMOVED:
			{
				const ::Vadon::Foundation::EditorModelMessageComponentRemoved* component_removed = reinterpret_cast<const ::Vadon::Foundation::EditorModelMessageComponentRemoved*>(message_data);
				process_scene_message(component_removed->scene_id, data, size);
			}
				break;
			}
		}
		break;
		}
	}

	void SceneSystem::process_scene_message(const ::Vadon::Foundation::UUID& scene_id_uuid, const char* data, size_t size)
	{
		const Vadon::Model::SceneID scene_id = Vadon::Model::SceneID::from_resource_id(scene_id_uuid);
		Scene* scene = find_scene(scene_id);
		
		VADON_ASSERT(scene != nullptr, "Cannot find scene");
		scene->process_message(data, size);
	}

	void SceneSystem::internal_remove_scene(Scene* scene)
	{
		auto scene_it = m_scene_lookup.find(scene->get_id());
		VADON_ASSERT(scene_it != m_scene_lookup.end(), "Cannot find scene in lookup");

		scene->close_scene();

		m_scene_lookup.erase(scene_it);
		delete scene;
	}
}