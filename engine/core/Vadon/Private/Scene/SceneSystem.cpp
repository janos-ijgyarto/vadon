#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <format>

namespace Vadon::Private::Scene
{
	namespace
	{
		void log_property_serialization_error(std::string_view property_name)
		{
			Vadon::Core::Logger::log_error(std::format("Scene system: unable to serialize property \"{}\"!\n", property_name));
		}

		std::string get_entity_path(const SceneData& scene, int32_t entity_index)
		{
			const SceneData::EntityData& entity_data = scene.entities[entity_index];
			if (entity_data.has_parent() == false)
			{
				return ".";
			}

			std::string path;

			const SceneData::EntityData* current_entity_data = &entity_data;
			while (current_entity_data->has_parent() == true)
			{
				path = current_entity_data->name + "/" + path;
				current_entity_data = &scene.entities[current_entity_data->parent];
			}

			// Trim a final slash
			path.pop_back();

			return path;
		}

		bool serialize_component(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, size_t index, SceneData::ComponentData& component_data)
		{
			constexpr const char* c_component_obj_error_message = "Scene system: unable to serialize component object!\n";

			if (serializer.open_object(index) == false)
			{
				Vadon::Core::Logger::log_error(c_component_obj_error_message);
				return false;
			}

			{
				std::string type_name;
				if (serializer.is_reading() == false)
				{
					type_name = Vadon::Utilities::TypeRegistry::get_type_info(component_data.type_id).name;
				}
				if (serializer.serialize("type", type_name) == false)
				{
					log_property_serialization_error("type");
					return false;
				}
				if (serializer.is_reading() == true)
				{
					component_data.type_id = Vadon::Utilities::TypeRegistry::get_type_id(type_name);
					if (component_data.type_id == Vadon::Utilities::c_invalid_type_id)
					{
						Vadon::Core::Logger::log_error(std::format("Scene system: loading component with unknown type \"{}\"!\n", type_name));
						return false;
					}
				}
			}

			if (serializer.open_object("properties") == false)
			{
				log_property_serialization_error("properties");
				return false;
			}

			if (serializer.is_reading() == true)
			{				
				// FIXME: this forces us to iterate over all properties, instead of just reading the ones present in the object
				// Need to iterate over the K/V pairs instead
				// FIXME2: we also need to make sure we discard properties not present in the object (i.e if the properties changed)
				const Vadon::Utilities::PropertyInfoList component_properties = Vadon::Utilities::TypeRegistry::get_type_properties(component_data.type_id);
				SceneData::ComponentData::Property property_data;
				for (const Vadon::Utilities::PropertyInfo& current_property : component_properties)
				{
					if (current_property.type_index == Vadon::Utilities::type_list_index_v<ResourceHandle, Vadon::Utilities::Variant>)
					{
						if (context.serialize_resource_property(serializer, current_property.name, property_data.value) == false)
						{
							return false;
						}
;
						property_data.name = current_property.name;
						component_data.properties.push_back(property_data);
						continue;
					}

					if (serializer.serialize(current_property.name, property_data.value) == true)
					{
						property_data.name = current_property.name;
						component_data.properties.push_back(property_data);
					}
				}
			}
			else
			{
				for (SceneData::ComponentData::Property& current_property : component_data.properties)
				{
					if (current_property.value.index() == Vadon::Utilities::type_list_index_v<ResourceHandle, Vadon::Utilities::Variant>)
					{
						if (context.serialize_resource_property(serializer, current_property.name, current_property.value) == false)
						{
							return false;
						}

						continue;
					}

					if (serializer.serialize(current_property.name, current_property.value) == false)
					{
						log_property_serialization_error(current_property.name);
						return false;
					}
				}
			}

			if (serializer.close_object() == false)
			{
				log_property_serialization_error("properties");
				return false;
			}

			if (serializer.close_object() == false)
			{
				Vadon::Core::Logger::log_error(c_component_obj_error_message);
				return false;
			}

			return true;
		}

		bool serialize_entity(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, size_t index, SceneData::EntityData& entity_data)
		{
			constexpr const char* c_entity_obj_error_message = "Scene system: unable to serialize entity object!\n";
			constexpr const char* c_component_array_error_message = "Scene system: unable to serialize component array!\n";

			if (serializer.open_object(index) == false)
			{
				Vadon::Core::Logger::log_error(c_entity_obj_error_message);
				return false;
			}
			serializer.serialize("name", entity_data.name);
			serializer.serialize("parent", entity_data.parent);
			// NOTE: scene needs to be handled separately because it may or may not be set
			{
				Vadon::Utilities::Variant scene_handle_variant;
				if (serializer.is_reading() == true)
				{
					Vadon::Utilities::Variant scene_id_variant;
					if (serializer.serialize("scene", scene_id_variant) == true)
					{
						entity_data.scene = context.load_resource(std::get<ResourceID>(scene_id_variant));
						if (entity_data.scene.is_valid() == false)
						{
							return false;
						}
					}
				}
				else
				{
					if (entity_data.scene.is_valid() == true)
					{
						scene_handle_variant = entity_data.scene;
						context.serialize_resource_property(serializer, "scene", scene_handle_variant);
					}
				}
			}

			if(serializer.open_array("components") == false)
			{
				Vadon::Core::Logger::log_error(c_component_array_error_message);
				return false;
			}

			const size_t component_count = serializer.is_reading() ? serializer.get_array_size() : entity_data.components.size();
			for (size_t current_component_index = 0; current_component_index < component_count; ++current_component_index)
			{
				SceneData::ComponentData& current_component_data = serializer.is_reading() ? entity_data.components.emplace_back() : entity_data.components[current_component_index];
				if (serialize_component(context, serializer, current_component_index, current_component_data) == false)
				{
					return false;
				}
			}

			if (serializer.close_array() == false)
			{
				Vadon::Core::Logger::log_error(c_component_array_error_message);
				return false;
			}
			if (serializer.close_object() == false)
			{
				Vadon::Core::Logger::log_error(c_entity_obj_error_message);
				return false;
			}

			return true;
		}
	}

	void SceneData::register_scene_type_info()
	{
		Vadon::Scene::ResourceRegistry::register_resource_type<Scene, ResourceBase>();
		Vadon::Scene::ResourceRegistry::register_resource_type<SceneData, Scene>();

		Vadon::ECS::ComponentRegistry::register_component_type<SceneComponent>();
	}

	ResourceHandle SceneSystem::create_scene()
	{
		return m_resource_system.create_resource<SceneData>();
	}

	bool SceneSystem::set_scene_data(ResourceHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity)
	{
		SceneData temp_scene_data;
		{
			// Track dependency stack (ensures we cannot save an invalid scene)
			std::vector<ResourceHandle> dependency_stack;
			dependency_stack.push_back(scene_handle);

			if (parse_scene_entity(ecs_world, root_entity, -1, temp_scene_data, dependency_stack) == false)
			{
				return false;
			}
		}

		SceneData* scene_data = m_resource_system.get_resource<SceneData>(scene_handle);
		scene_data->swap(temp_scene_data);

		return true;
	}

	ECS::EntityHandle SceneSystem::instantiate_scene(ResourceHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene)
	{
		// FIXME: should we perform circular dependency validation?
		// In principle we cannot have an invalid scene by this stage
		const SceneData* scene_data = m_resource_system.get_resource<SceneData>(scene_handle);

		std::vector<Vadon::ECS::EntityHandle> entity_lookup;

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		for (const SceneData::EntityData& current_entity_data : scene_data->entities)
		{
			Vadon::ECS::EntityHandle current_entity;
			if (current_entity_data.scene.is_valid() == false)
			{
				current_entity = entity_manager.create_entity();
			}
			else
			{
				current_entity = instantiate_scene(current_entity_data.scene, ecs_world, true);
			}

			entity_manager.set_entity_name(current_entity, current_entity_data.name);

			for (const SceneData::ComponentData& current_component_data : current_entity_data.components)
			{
				void* current_component = nullptr;
				if (current_entity_data.scene.is_valid() == false)
				{
					current_component = component_manager.add_component(current_entity, current_component_data.type_id);
				}
				else
				{
					current_component = component_manager.get_component(current_entity, current_component_data.type_id);
				}

				if (current_component == nullptr)
				{
					// TODO: error?
					continue;
				}

				// TODO: if property is resource, handle separately!
				for (const SceneData::ComponentData::Property& current_property_data : current_component_data.properties)
				{
					Vadon::Utilities::TypeRegistry::set_property(current_component, current_component_data.type_id, current_property_data.name, current_property_data.value);
				}
			}

			entity_lookup.push_back(current_entity);

			if (current_entity_data.has_parent() == true)
			{
				Vadon::ECS::EntityHandle parent_entity = entity_lookup[current_entity_data.parent];
				entity_manager.add_child_entity(parent_entity, current_entity);
			}
		}

		const Vadon::ECS::EntityHandle root_entity = entity_lookup.front();
		if (is_sub_scene == true)
		{
			// Add scene component
			SceneComponent& root_scene_component = component_manager.add_component<SceneComponent>(root_entity);
			root_scene_component.root_scene = scene_handle;

			// Add scene component to each child, indicates that these were instantiated from the scene
			for (Vadon::ECS::EntityHandle current_child : entity_manager.get_children(root_entity))
			{
				SceneComponent* child_scene_component = component_manager.get_component<SceneComponent>(current_child);
				if (child_scene_component == nullptr)
				{
					child_scene_component = &component_manager.add_component<SceneComponent>(current_child);
				}
				child_scene_component->parent_scene = scene_handle;
			}
		}

		return root_entity;
	}

	bool SceneSystem::is_scene_dependent(ResourceHandle scene_handle, ResourceHandle dependent_scene_handle) const
	{
		std::vector<ResourceHandle> dependency_stack;
		dependency_stack.push_back(scene_handle);
		if (internal_is_scene_dependent(dependent_scene_handle, dependency_stack) == true)
		{
			return true;
		}

		return false;
	}

	bool SceneSystem::serialize_scene(Vadon::Scene::ResourceSystemInterface& context, Vadon::Utilities::Serializer& serializer, ResourceBase& resource)
	{
		constexpr const char* c_entity_array_error_log = "Scene system: unable to serialize component object!\n";
		SceneData& scene_data = static_cast<SceneData&>(resource);

		if (serializer.open_array("entities") == false)
		{
			Vadon::Core::Logger::log_error(c_entity_array_error_log);
			return false;
		}

		// FIXME: all these members should be possible to serialize just using Variant!
		// Resource IDs may be the exception, need to have a way to resolve them
		if (serializer.is_reading() == true)
		{
			const size_t entity_count = serializer.get_array_size();
			for (size_t current_entity_index = 0; current_entity_index < entity_count; ++current_entity_index)
			{
				SceneData::EntityData& current_entity_data = scene_data.entities.emplace_back();
				if (serialize_entity(context, serializer, current_entity_index, current_entity_data) == false)
				{
					return false;
				}
			}
		}
		else
		{
			for (size_t current_entity_index = 0; current_entity_index < scene_data.entities.size(); ++current_entity_index)
			{
				if (serialize_entity(context, serializer, current_entity_index, scene_data.entities[current_entity_index]) == false)
				{
					return false;
				}
			}
		}

		if (serializer.close_array() == false)
		{
			Vadon::Core::Logger::log_error(c_entity_array_error_log);
			return false;
		}

		return true;
	}

	SceneSystem::SceneSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::SceneSystem(core)
		, m_resource_system(core)
	{}

	bool SceneSystem::initialize()
	{
		log_message("Initializing Scene System\n");
		if (m_resource_system.initialize() == false)
		{
			return false;
		}

		SceneData::register_scene_type_info();
		Vadon::Scene::ResourceRegistry::register_serializer<SceneData>(&serialize_scene);
		log_message("Scene System initialized!\n");
		return true;
	}

	void SceneSystem::shutdown()
	{
		log_message("Shutting down Scene System\n");
		m_resource_system.shutdown();
		log_message("Scene System shut down!\n");
	}

	bool SceneSystem::parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<ResourceHandle>& dependency_stack)
	{
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		const SceneComponent* scene_comp = component_manager.get_component<SceneComponent>(entity);
		if (scene_comp != nullptr)
		{
			if (scene_comp->parent_scene.is_valid() == true)
			{
				// Child is part of instantiated scene
				return true;
			}
		}

		const int32_t entity_index = static_cast<int32_t>(scene_data.entities.size());
		SceneData::EntityData& entity_data = scene_data.entities.emplace_back();

		entity_data.name = entity_manager.get_entity_name(entity);
		entity_data.parent = parent_index;

		if (scene_comp != nullptr)
		{
			if (scene_comp->root_scene.is_valid() == false)
			{
				// FIXME: print entity metadata
				Vadon::Core::Logger::log_error("Scene system: Entity has Scene Component but no valid metadata!\n");
				return false;
			}

			// Make sure we don't have a circular dependency
			// FIXME: use LUT to make sure we don't check more than once?
			if (internal_is_scene_dependent(scene_comp->root_scene, dependency_stack) == true)
			{
				Vadon::Core::Logger::log_error("Scene system: parsed scene has circular dependency!\n");
				return false;
			}

			entity_data.scene = scene_comp->root_scene;
		}

		const Vadon::ECS::ComponentIDList component_type_ids = component_manager.get_component_list(entity);
		const Vadon::Utilities::TypeID scene_component_id = Vadon::Utilities::TypeRegistry::get_type_id<SceneComponent>();
		for (Vadon::Utilities::TypeID current_component_type_id : component_type_ids)
		{
			if (current_component_type_id == scene_component_id)
			{
				continue;
			}

			SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
			current_component_data.type_id = current_component_type_id;

			void* component_ptr = component_manager.get_component(entity, current_component_type_id);
			const Vadon::Utilities::PropertyList component_properties = Vadon::Utilities::TypeRegistry::get_properties(component_ptr, current_component_type_id);

			// TODO: if property is resource, handle separately!

			// FIXME: filter to properties that are intended to be serialized?
			for (const Vadon::Utilities::Property& current_component_property : component_properties)
			{
				SceneData::ComponentData::Property& current_property_data = current_component_data.properties.emplace_back();
				current_property_data.name = current_component_property.name;
				current_property_data.value = current_component_property.value;
			}
		}

		for (Vadon::ECS::EntityHandle current_child : entity_manager.get_children(entity))
		{
			if (parse_scene_entity(ecs_world, current_child, entity_index, scene_data, dependency_stack) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool SceneSystem::internal_is_scene_dependent(ResourceHandle scene_handle, std::vector<ResourceHandle>& dependency_stack) const
	{
		if (std::find(dependency_stack.begin(), dependency_stack.end(), scene_handle) != dependency_stack.end())
		{
			return true;
		}

		// Add to stack, recursively check entities if they might lead to a circular dependency
		dependency_stack.push_back(scene_handle);

		const SceneData* scene_data = m_resource_system.get_resource<SceneData>(scene_handle);
		for (const SceneData::EntityData& current_entity_data : scene_data->entities)
		{
			if (current_entity_data.scene.is_valid() == true)
			{
				if (internal_is_scene_dependent(current_entity_data.scene, dependency_stack) == true)
				{
					return true;
				}
			}
		}
		dependency_stack.pop_back();

		return false;
	}
}