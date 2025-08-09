#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

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

		bool save_scene_array_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::Variant& array_value)
		{
			using SerializerResult = Vadon::Utilities::Serializer::Result;
			using ErasedDataType = Vadon::Utilities::ErasedDataType;

			bool successful = true;
			static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::Variant& value, Vadon::Utilities::ErasedDataTypeID data_type)
				{
					const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, index, value, data_type);
					if (result != SerializerResult::SUCCESSFUL)
					{
						return false;
					}

					return true;
				};

			Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
			for (size_t index = 0; index < array.data.size(); ++index)
			{
				Vadon::Utilities::Variant& current_value = array.data[index];
				switch (array.data_type.type)
				{
				case ErasedDataType::TRIVIAL:
					successful &= c_process_trivial_property(serializer, index, current_value, array.data_type);
					break;
				case ErasedDataType::RESOURCE_ID:
					successful &= c_process_trivial_property(serializer, index, current_value, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>());
					break;
				default:
					VADON_UNREACHABLE;
					break;
				}
			}

			return successful;
		}

		bool load_scene_array_data(Vadon::Utilities::Serializer& serializer, Vadon::Utilities::Variant& array_value)
		{
			using SerializerResult = Vadon::Utilities::Serializer::Result;
			using ErasedDataType = Vadon::Utilities::ErasedDataType;

			Vadon::Utilities::VariantArray& array = *std::get<Vadon::Utilities::BoxedVariantArray>(array_value);
			array.data.clear();

			bool successful = true;
			static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, size_t index, Vadon::Utilities::Variant& value, Vadon::Utilities::ErasedDataTypeID data_type)
				{
					const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, index, value, data_type);
					if (result != SerializerResult::SUCCESSFUL)
					{
						return false;
					}

					return true;
				};

			Vadon::Utilities::Variant current_value;
			const size_t array_size = serializer.get_array_size();
			for (size_t index = 0; index < array_size; ++index)
			{
				switch (array.data_type.type)
				{
				case ErasedDataType::TRIVIAL:
					successful &= c_process_trivial_property(serializer, index, current_value, array.data_type);
					break;
				case ErasedDataType::RESOURCE_ID:
					successful &= c_process_trivial_property(serializer, index, current_value, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>());
					break;
				default:
					VADON_UNREACHABLE;
					break;
				}

				array.data.push_back(current_value);
			}

			return successful;
		}

		bool serialize_component(Vadon::Utilities::Serializer& serializer, size_t index, SceneData::ComponentData& component_data)
		{
			constexpr const char* c_component_obj_error_message = "Scene system: unable to serialize component object!\n";

			using SerializerResult = Vadon::Utilities::Serializer::Result;
			if (serializer.open_object(index) != SerializerResult::SUCCESSFUL)
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
				if (serializer.serialize("type", type_name) != SerializerResult::SUCCESSFUL)
				{
					log_property_serialization_error("type");
					return false;
				}
				if (serializer.is_reading() == true)
				{
					component_data.type_id = Vadon::Utilities::TypeRegistry::get_type_id(type_name);
					if (component_data.type_id == Vadon::Utilities::TypeID::INVALID)
					{
						Vadon::Core::Logger::log_error(std::format("Scene system: loading component with unknown type \"{}\"!\n", type_name));
						return false;
					}
				}
			}

			if (serializer.open_object("properties") != SerializerResult::SUCCESSFUL)
			{
				log_property_serialization_error("properties");
				return false;
			}

			using ErasedDataType = Vadon::Utilities::ErasedDataType;
			if (serializer.is_reading() == true)
			{	
				static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, SceneData::ComponentData& component_data, std::string_view key, Vadon::Utilities::Property& property_data, Vadon::Utilities::ErasedDataTypeID data_type)
					{
						const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, key, property_data.value, data_type);
						if (result == SerializerResult::SUCCESSFUL)
						{
							property_data.name = key;
							property_data.data_type = data_type;
							component_data.properties.push_back(property_data);
						}
						else
						{
							// TODO: log error?
						}
					};

				// FIXME: this forces us to iterate over all properties, instead of just reading the ones present in the object
				// Need to iterate over the K/V pairs instead
				// FIXME2: we also need to make sure we discard properties not present in the object (i.e if the properties changed)
				const Vadon::Utilities::PropertyInfoList component_properties = Vadon::Utilities::TypeRegistry::get_type_properties(component_data.type_id);
				Vadon::Utilities::Property property_data;
				for (const Vadon::Utilities::PropertyInfo& current_property : component_properties)
				{
					// Check if key is present (if not, assume we should just use default value)
					// FIXME: invert this to instead only process keys that are actually in the data?
					if (serializer.has_key(current_property.name) == false)
					{
						continue;
					}

					switch (current_property.data_type.type)
					{
					case ErasedDataType::TRIVIAL:
						c_process_trivial_property(serializer, component_data, current_property.name, property_data, current_property.data_type);
						break;
					case ErasedDataType::RESOURCE_ID:
						c_process_trivial_property(serializer, component_data, current_property.name, property_data, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>());
						break;
					case ErasedDataType::ARRAY:
					{
						if (serializer.open_array(current_property.name) != SerializerResult::SUCCESSFUL)
						{
							log_property_serialization_error(current_property.name);
							return false;
						}

						// First get default value from Component registry
						property_data.value = Vadon::ECS::ComponentRegistry::get_component_property_default_value(component_data.type_id, current_property.name);

						// Use default value to determine how to deserialize the data
						if (load_scene_array_data(serializer, property_data.value) == true)
						{
							property_data.name = current_property.name;
							property_data.data_type = current_property.data_type;
							component_data.properties.push_back(property_data);
						}
						else
						{
							// TODO: error?
							log_property_serialization_error(current_property.name);
						}

						if (serializer.close_array() != SerializerResult::SUCCESSFUL)
						{
							log_property_serialization_error(current_property.name);
							return false;
						}
					}
					break;
					}
				}
			}
			else
			{
				static constexpr auto c_process_trivial_property = +[](Vadon::Utilities::Serializer& serializer, std::string_view key, Vadon::Utilities::Property& property_data, Vadon::Utilities::ErasedDataTypeID data_type)
					{
						const SerializerResult result = Vadon::Utilities::process_trivial_property(serializer, key, property_data.value, data_type);
						if (result != SerializerResult::SUCCESSFUL)
						{
							// TODO: log error?
						}
					};

				for (Vadon::Utilities::Property& current_property : component_data.properties)
				{
					switch (current_property.data_type.type)
					{
					case ErasedDataType::TRIVIAL:
					c_process_trivial_property(serializer, current_property.name, current_property, current_property.data_type);
					break;
					case ErasedDataType::RESOURCE_ID:
					c_process_trivial_property(serializer, current_property.name, current_property, Vadon::Utilities::get_erased_data_type_id<Vadon::Utilities::UUID>());
					break;
					case ErasedDataType::ARRAY:
					{
						if (serializer.open_array(current_property.name) != SerializerResult::SUCCESSFUL)
						{
							log_property_serialization_error(current_property.name);
							return false;
						}

						if (save_scene_array_data(serializer, current_property.value) == false)
						{
							// TODO: error?
						}

						if (serializer.close_array() != SerializerResult::SUCCESSFUL)
						{
							log_property_serialization_error(current_property.name);
							return false;
						}
					}
					break;
					}
				}
			}

			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				log_property_serialization_error("properties");
				return false;
			}

			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_component_obj_error_message);
				return false;
			}

			return true;
		}

		bool serialize_entity(Vadon::Utilities::Serializer& serializer, size_t index, SceneData::EntityData& entity_data)
		{
			constexpr const char* c_entity_obj_error_message = "Scene system: unable to serialize entity object!\n";
			constexpr const char* c_component_array_error_message = "Scene system: unable to serialize component array!\n";

			using SerializerResult = Vadon::Utilities::Serializer::Result;
			if (serializer.open_object(index) != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_entity_obj_error_message);
				return false;
			}
			serializer.serialize("name", entity_data.name);
			serializer.serialize("parent", entity_data.parent);
			// NOTE: scene needs to be handled separately because it may or may not be set
			if ((serializer.is_reading() == true) || ((serializer.is_reading() == false) && (entity_data.scene.is_valid() == true)))
			{
				serializer.serialize("scene", entity_data.scene.as_resource_id());
			}

			if(serializer.open_array("components") != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_component_array_error_message);
				return false;
			}

			const size_t component_count = serializer.is_reading() ? serializer.get_array_size() : entity_data.components.size();
			for (size_t current_component_index = 0; current_component_index < component_count; ++current_component_index)
			{
				if (serializer.is_reading() == true)
				{
					SceneData::ComponentData current_component_data;
					if (serialize_component(serializer, current_component_index, current_component_data) == true)
					{
						entity_data.components.push_back(current_component_data);
					}
					else
					{
						// TODO: have a "pedantic" mode where we early out if there are any errors?
					}
				}
				else
				{
					SceneData::ComponentData& current_component_data = entity_data.components[current_component_index];
					if (serialize_component(serializer, current_component_index, current_component_data) == false)
					{
						// TODO: have a "pedantic" mode where we early out if there are any errors?
					}
				}
			}

			if (serializer.close_array() != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_component_array_error_message);
				return false;
			}
			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_entity_obj_error_message);
				return false;
			}

			return true;
		}

		bool serialize_scene(Vadon::Scene::ResourceSystem& resource_system, Vadon::Utilities::Serializer& serializer, Resource& resource)
		{
			using SerializerResult = Vadon::Utilities::Serializer::Result;
			constexpr const char* c_entity_array_error_log = "Scene system: unable to serialize component object!\n";

			Scene& scene = static_cast<Scene&>(resource);

			if (serializer.open_array("entities") != SerializerResult::SUCCESSFUL)
			{
				resource_system.log_error(c_entity_array_error_log);
				return false;
			}

			if (serializer.is_reading() == true)
			{
				const size_t entity_count = serializer.get_array_size();
				for (size_t current_entity_index = 0; current_entity_index < entity_count; ++current_entity_index)
				{
					SceneData::EntityData& current_entity_data = scene.data.entities.emplace_back();
					if (serialize_entity(serializer, current_entity_index, current_entity_data) == false)
					{
						return false;
					}
				}
			}
			else
			{
				for (size_t current_entity_index = 0; current_entity_index < scene.data.entities.size(); ++current_entity_index)
				{
					if (serialize_entity(serializer, current_entity_index, scene.data.entities[current_entity_index]) == false)
					{
						return false;
					}
				}
			}

			if (serializer.close_array() != SerializerResult::SUCCESSFUL)
			{
				resource_system.log_error(c_entity_array_error_log);
				return false;
			}

			return true;
		}
	}

	void SceneData::register_scene_type_info()
	{
		Vadon::Scene::ResourceRegistry::register_resource_type<Vadon::Scene::Scene, Resource>();

		Vadon::Scene::ResourceRegistry::register_resource_serializer<Scene>(&serialize_scene);

		Vadon::ECS::ComponentRegistry::register_component_type<SceneComponent>();
	}

	SceneHandle SceneSystem::create_scene()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		SceneHandle new_scene_handle = SceneHandle::from_resource_handle(resource_system.create_resource<Scene>());

		if (new_scene_handle.is_valid() == false)
		{
			log_error("Scene system: failed to create scene!\n");
			return new_scene_handle;
		}

		return new_scene_handle;
	}

	SceneHandle SceneSystem::find_scene(SceneID scene_id) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const ResourceHandle resource_handle = resource_system.find_resource(scene_id);
		if (resource_handle.is_valid() == false)
		{
			return SceneHandle();
		}

		if (resource_system.get_resource_info(resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Scene>())
		{
			// TODO: log resource ID!
			log_error("Scene system: resource ID does not correspond to scene resource!\n");
			return SceneHandle();
		}

		return SceneHandle::from_resource_handle(resource_handle);
	}

	SceneHandle SceneSystem::load_scene(SceneID scene_id)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const ResourceHandle scene_resource_handle = resource_system.load_resource(scene_id);
		if (resource_system.get_resource_info(scene_resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Scene>())
		{
			// TODO: log resource ID!
			// TODO2: remove loaded resource?
			log_error("Scene system: resource ID does not correspond to scene resource!\n");
			return SceneHandle();
		}
		return SceneHandle::from_resource_handle(scene_resource_handle);
	}

	bool SceneSystem::package_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		Scene* scene = resource_system.get_resource<Scene>(scene_handle);

		// Use temp object that we can discard if something goes wrong
		SceneData temp_scene_data;
		{
			// Track dependency stack (ensures we cannot save an invalid scene)
			std::vector<SceneID> dependency_stack;
			const SceneID scene_id = SceneID::from_resource_id(resource_system.get_resource_info(scene_handle).id);
			dependency_stack.push_back(scene_id);

			if (parse_scene_entity(ecs_world, root_entity, -1, temp_scene_data, dependency_stack) == false)
			{
				return false;
			}
		}

		// Swap temp data into scene resource
		scene->data.swap(temp_scene_data);
		return true;
	}

	ECS::EntityHandle SceneSystem::instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene)
	{
		// TODO: circular dependency check?
		const Scene* scene = get_scene(scene_handle);
		std::vector<Vadon::ECS::EntityHandle> entity_lookup;

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		for (const SceneData::EntityData& current_entity_data : scene->data.entities)
		{
			Vadon::ECS::EntityHandle current_entity;
			if (current_entity_data.scene.is_valid() == false)
			{
				current_entity = entity_manager.create_entity();
			}
			else
			{
				const SceneHandle sub_scene_handle = load_scene(current_entity_data.scene);
				if (sub_scene_handle.is_valid() == false)
				{
					// TODO: log scene ID!
					// TODO2: should we abort, or just skip the entity that failed to load?
					log_error("Scene system: failed to load sub-scene while instantiating!\n");
					if (entity_lookup.empty() == false)
					{
						// Clean up from root
						const Vadon::ECS::EntityHandle root_entity = entity_lookup.front();
						ecs_world.get_entity_manager().remove_entity(root_entity);
						ecs_world.remove_pending_entities();
					}

					return Vadon::ECS::EntityHandle();
				}
				current_entity = instantiate_scene(sub_scene_handle, ecs_world, true);
				if (current_entity.is_valid() == false)
				{
					if (entity_lookup.empty() == false)
					{
						// Clean up from root
						const Vadon::ECS::EntityHandle root_entity = entity_lookup.front();
						ecs_world.get_entity_manager().remove_entity(root_entity);
						ecs_world.remove_pending_entities();
					}

					return Vadon::ECS::EntityHandle();
				}
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

				using ErasedDataType = Vadon::Utilities::ErasedDataType;
				for (const Vadon::Utilities::Property& current_property_data : current_component_data.properties)
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
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			const SceneID scene_id = SceneID::from_resource_id(resource_system.get_resource_info(scene_handle).id);

			SceneComponent& root_scene_component = component_manager.add_component<SceneComponent>(root_entity);
			root_scene_component.root_scene = scene_id;

			// Add scene component to each child, indicates that these were instantiated from the scene
			for (Vadon::ECS::EntityHandle current_child : entity_manager.get_children(root_entity))
			{
				SceneComponent* child_scene_component = component_manager.get_component<SceneComponent>(current_child);
				if (child_scene_component == nullptr)
				{
					child_scene_component = &component_manager.add_component<SceneComponent>(current_child);
				}
				child_scene_component->parent_scene = scene_id;
			}
		}	

		return root_entity;
	}

	bool SceneSystem::is_scene_dependent(SceneID base_scene_id, SceneID dependent_scene_id)
	{
		if (base_scene_id == dependent_scene_id)
		{
			return true;
		}

		std::vector<SceneID> dependency_stack;
		dependency_stack.push_back(base_scene_id);
		
		return internal_is_scene_dependent(dependent_scene_id, dependency_stack);
	}

	SceneSystem::SceneSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::SceneSystem(core)
	{}

	bool SceneSystem::initialize()
	{
		log_message("Initializing Scene System\n");
		SceneData::register_scene_type_info();
		log_message("Scene System initialized!\n");
		return true;
	}

	void SceneSystem::shutdown()
	{
		log_message("Shutting down Scene System\n");
		// TODO: anything?
		log_message("Scene System shut down!\n");
	}

	bool SceneSystem::parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<SceneID>& dependency_stack)
	{
		using ErasedDataType = Vadon::Utilities::ErasedDataType;

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

		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const Vadon::ECS::ComponentIDList component_type_ids = component_manager.get_component_list(entity);
		const Vadon::Utilities::TypeID scene_component_id = Vadon::Utilities::TypeRegistry::get_type_id<SceneComponent>();

		static constexpr auto c_save_property_func = +[](Vadon::Utilities::Property& property_data, const Vadon::Utilities::Property& component_property)
			{
				property_data.name = component_property.name;
				property_data.data_type = component_property.data_type;
				property_data.value = component_property.value;
			};

		if (scene_comp != nullptr)
		{
			if (scene_comp->root_scene.is_valid() == false)
			{
				// FIXME: print entity metadata
				log_error("Scene system: Entity has Scene Component but no valid metadata!\n");
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

			const SceneHandle sub_scene_handle = load_scene(scene_comp->root_scene);
			if (sub_scene_handle.is_valid() == false)
			{
				// TODO: log scene ID!
				log_error("Scene system: failed to load sub-scene!\n");
				return true;
			}

			// Compare against root entity
			const Scene* sub_scene = resource_system.get_resource<Scene>(sub_scene_handle);
			const SceneData::EntityData& sub_scene_root = sub_scene->data.entities.front();
			std::vector<size_t> unique_property_indices;
			for (Vadon::Utilities::TypeID current_component_type_id : component_type_ids)
			{
				if (current_component_type_id == scene_component_id)
				{
					continue;
				}

				void* component_ptr = component_manager.get_component(entity, current_component_type_id);
				const Vadon::Utilities::PropertyList component_properties = Vadon::Utilities::TypeRegistry::get_properties(component_ptr, current_component_type_id);

				bool found = false;
				for (const SceneData::ComponentData& sub_scene_component : sub_scene_root.components)
				{
					if (sub_scene_component.type_id == current_component_type_id)
					{
						unique_property_indices.clear();
						for (size_t property_index = 0; property_index < component_properties.size(); ++property_index)
						{
							const Vadon::Utilities::Property& current_property_data = component_properties[property_index];
							for (const Vadon::Utilities::Property& sub_scene_property : sub_scene_component.properties)
							{
								if (current_property_data.name == sub_scene_property.name)
								{
									if (current_property_data.value != sub_scene_property.value)
									{
										unique_property_indices.push_back(property_index);
									}
									break;
								}
							}
						}

						if (unique_property_indices.empty() == false)
						{
							// Save the unique properties
							SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
							current_component_data.type_id = current_component_type_id;

							for (size_t current_property_index : unique_property_indices)
							{
								Vadon::Utilities::Property& current_property_data = current_component_data.properties.emplace_back();
								const Vadon::Utilities::Property& component_property = component_properties[current_property_index];
								
								c_save_property_func(current_property_data, component_property);
							}
						}
						found = true;
						break;
					}
				}

				if (found == false)
				{
					// Component was not in original, save entirely
					SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
					current_component_data.type_id = current_component_type_id;

					// FIXME: filter to properties that are intended to be serialized?
					for (const Vadon::Utilities::Property& current_component_property : component_properties)
					{
						Vadon::Utilities::Property& current_property_data = current_component_data.properties.emplace_back();
						c_save_property_func(current_property_data, current_component_property);
					}
				}
			}
		}
		else
		{
			for (Vadon::Utilities::TypeID current_component_type_id : component_type_ids)
			{
				if (current_component_type_id == scene_component_id)
				{
					continue;
				}

				void* component_ptr = component_manager.get_component(entity, current_component_type_id);
				const Vadon::Utilities::PropertyList component_properties = Vadon::Utilities::TypeRegistry::get_properties(component_ptr, current_component_type_id);

				SceneData::ComponentData& current_component_data = entity_data.components.emplace_back();
				current_component_data.type_id = current_component_type_id;

				// FIXME: filter to properties that are intended to be serialized?
				for (const Vadon::Utilities::Property& current_component_property : component_properties)
				{
					Vadon::Utilities::Property& current_property_data = current_component_data.properties.emplace_back();
					c_save_property_func(current_property_data, current_component_property);
				}
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

	bool SceneSystem::internal_is_scene_dependent(SceneID scene_id, std::vector<SceneID>& dependency_stack)
	{
		if (std::find(dependency_stack.begin(), dependency_stack.end(), scene_id) != dependency_stack.end())
		{
			return true;
		}

		// Add to stack, recursively check entities if they might lead to a circular dependency
		dependency_stack.push_back(scene_id);

		const SceneHandle scene_handle = load_scene(scene_id);
		if (scene_handle.is_valid() == false)
		{
			// TODO: log scene ID!
			log_error("Scene system: failed to load scene during dependency check!\n");
			return true;
		}

		const Scene* scene = get_scene(scene_handle);
		for (const SceneData::EntityData& current_entity_data : scene->data.entities)
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

	const Scene* SceneSystem::get_scene(SceneHandle scene_handle) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		return resource_system.get_resource<Scene>(scene_handle);
	}
}