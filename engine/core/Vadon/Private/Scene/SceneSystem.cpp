#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

namespace Vadon::Private::Scene
{
	namespace
	{
		bool parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data)
		{
			const int32_t entity_index = static_cast<int32_t>(scene_data.entities.size());
			SceneData::EntityData& entity_data = scene_data.entities.emplace_back();

			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();

			entity_data.name = entity_manager.get_entity_name(entity);
			entity_data.parent = parent_index;

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			const std::vector<uint32_t> component_type_ids = component_manager.get_component_list(entity);
			for (uint32_t current_component_type_id : component_type_ids)
			{
				SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
				current_component_data.type = Vadon::Utilities::TypeRegistry::get_type_info(current_component_type_id).name;

				void* component_ptr = component_manager.get_component(entity, current_component_type_id);
				const Vadon::Utilities::PropertyList component_properties = Vadon::Utilities::TypeRegistry::get_properties(component_ptr, current_component_type_id);

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
				if (parse_scene_entity(ecs_world, current_child, entity_index, scene_data) == false)
				{
					return false;
				}
			}

			return true;
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

		bool serialize_component(Vadon::Utilities::Serializer& serializer, size_t index, SceneData::ComponentData& component_data)
		{
			if (serializer.open_object(index) == false)
			{
				// TODO: error?
				return false;
			}

			if (serializer.serialize("type", component_data.type) == false)
			{
				// TODO: error?
				return false;
			}

			if (serializer.open_object("properties") == false)
			{
				// TODO: error?
				return false;
			}

			if (serializer.is_reading() == true)
			{
				// FIXME: this forces us to iterate over all properties, instead of just reading the ones present in the object
				// Need to iterate over the K/V pairs instead
				// FIXME2: we also need to make sure we discard properties not present in the object (i.e if the properties changed)
				const Vadon::Utilities::PropertyInfoList component_properties = Vadon::Utilities::TypeRegistry::get_type_properties(Vadon::Utilities::TypeRegistry::get_type_id(component_data.type));
				SceneData::ComponentData::Property property_data;
				for (const Vadon::Utilities::PropertyInfo& current_property : component_properties)
				{
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
					if (serializer.serialize(current_property.name, current_property.value) == false)
					{
						// TODO: error?
						return false;
					}
				}
			}

			if (serializer.close_object() == false)
			{
				// TODO: error?
				return false;
			}

			if (serializer.close_object() == false)
			{
				// TODO: error?
				return false;
			}

			return true;
		}

		bool serialize_entity(Vadon::Utilities::Serializer& serializer, size_t index, SceneData::EntityData& entity_data)
		{
			if (serializer.open_object(index) == false)
			{
				// TODO: error?
				return false;
			}
			serializer.serialize("name", entity_data.name);
			serializer.serialize("parent", entity_data.parent);
			if(serializer.open_array("components") == false)
			{
				// TODO: error?
				return false;
			}

			if (serializer.is_reading() == true)
			{
				const size_t component_count = serializer.get_array_size();
				for (size_t current_component_index = 0; current_component_index < component_count; ++current_component_index)
				{
					SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
					if (serialize_component(serializer, current_component_index, current_component_data) == false)
					{
						// TODO: error?
						return false;
					}
				}
			}
			else
			{
				for (size_t current_component_index = 0; current_component_index < entity_data.components.size(); ++current_component_index)
				{
					SceneData::ComponentData& current_component_data = entity_data.components[current_component_index];
					if (serialize_component(serializer, current_component_index, current_component_data) == false)
					{
						// TODO: error?
						return false;
					}
				}
			}
			if (serializer.close_array() == false)
			{
				// TODO: error?
				return false;
			}
			if (serializer.close_object() == false)
			{
				// TODO: error?
				return false;
			}

			return true;
		}
	}

	SceneHandle SceneSystem::create_scene(SceneInfo scene_info)
	{
		SceneHandle new_scene_handle = m_scene_pool.add();

		SceneData& scene_data = m_scene_pool.get(new_scene_handle);
		scene_data.info = scene_info;

		return new_scene_handle;
	}

	void SceneSystem::remove_scene(SceneHandle scene_handle)
	{
		// TODO: any cleanup?
		m_scene_pool.remove(scene_handle);
	}

	SceneInfo SceneSystem::get_scene_info(SceneHandle scene_handle) const
	{
		const SceneData& scene_data = m_scene_pool.get(scene_handle);
		return scene_data.info;
	}

	bool SceneSystem::set_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity)
	{
		SceneData& scene_data = m_scene_pool.get(scene_handle);
		scene_data.clear();

		return parse_scene_entity(ecs_world, root_entity, -1, scene_data);
	}

	ECS::EntityHandle SceneSystem::instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world)
	{
		const SceneData& scene_data = m_scene_pool.get(scene_handle);

		std::vector<Vadon::ECS::EntityHandle> entity_lookup;

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		for (const SceneData::EntityData& current_entity_data : scene_data.entities)
		{
			Vadon::ECS::EntityHandle current_entity = entity_manager.create_entity();
			entity_manager.set_entity_name(current_entity, current_entity_data.name);

			for (const SceneData::ComponentData& current_component_data : current_entity_data.components)
			{
				const uint32_t component_type_id = Vadon::Utilities::TypeRegistry::get_type_id(current_component_data.type);
				void* current_component = component_manager.add_component(current_entity, component_type_id);
				if (current_component == nullptr)
				{
					// TODO: error?
					continue;
				}

				for (const SceneData::ComponentData::Property& current_property_data : current_component_data.properties)
				{
					Vadon::Utilities::TypeRegistry::set_property(current_component, component_type_id, current_property_data.name, current_property_data.value);
				}
			}

			entity_lookup.push_back(current_entity);

			if (current_entity_data.has_parent() == true)
			{
				Vadon::ECS::EntityHandle parent_entity = entity_lookup[current_entity_data.parent];
				entity_manager.add_child_entity(parent_entity, current_entity);
			}
		}

		return entity_lookup.front();
	}

	bool SceneSystem::serialize_scene(SceneHandle scene_handle, Vadon::Utilities::Serializer& serializer)
	{
		if (serializer.is_reading() == true)
		{
			SceneData loaded_scene_data;
			serializer.serialize("name", loaded_scene_data.info.name);

			if (serializer.open_array("entities") == false)
			{
				// TODO: error?
				return false;
			}

			const size_t entity_count = serializer.get_array_size();
			for (size_t current_entity_index = 0; current_entity_index < entity_count; ++current_entity_index)
			{
				SceneData::EntityData& current_entity_data = loaded_scene_data.entities.emplace_back();
				if (serialize_entity(serializer, current_entity_index, current_entity_data) == false)
				{
					// TODO: error?
					return false;
				}
			}

			if (serializer.close_array() == false)
			{
				// TODO: error?
				return false;
			}

			// Only clear and swap once the entire load succeeded
			SceneData& scene_data = m_scene_pool.get(scene_handle);
			scene_data.swap(loaded_scene_data);
		}
		else
		{
			SceneData& scene_data = m_scene_pool.get(scene_handle);
			serializer.serialize("name", scene_data.info.name);

			if (serializer.open_array("entities") == false)
			{
				// TODO: error?
				return false;
			}

			for (size_t current_entity_index = 0; current_entity_index < scene_data.entities.size(); ++current_entity_index)
			{
				if (serialize_entity(serializer, current_entity_index, scene_data.entities[current_entity_index]) == false)
				{
					// TODO: error?
					return false;
				}
			}

			if (serializer.close_array() == false)
			{
				// TODO: error?
				return false;
			}
		}

		return true;
	}

	SceneSystem::SceneSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::SceneSystem(core)
		, m_resource_system(core)
	{}

	bool SceneSystem::initialize()
	{
		if (m_resource_system.initialize() == false)
		{
			return false;
		}

		return true;
	}
}