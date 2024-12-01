#include <Vadon/Private/PCH/Core.hpp>
#include <Vadon/Private/Scene/SceneSystem.hpp>

#include <Vadon/ECS/Component/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/FunctionBind.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/Utilities/Serialization/Serializer.hpp>

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
		
		template<typename T>
		constexpr size_t variant_type_list_index_v = Vadon::Utilities::type_list_index_v<T, Vadon::Utilities::Variant>;

		// TODO: default value?
		template<typename T>
		bool serialize_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::Serializer::Result& result, Vadon::Utilities::ErasedDataTypeID data_type)
		{
			if (data_type.id != variant_type_list_index_v<T>)
			{
				return false;
			}

			if (serializer.is_reading() == true)
			{
				// TODO: allow setting default value if not found?
				T value;
				result = serializer.serialize(property_name, value);
				if (result == Vadon::Utilities::Serializer::Result::SUCCESSFUL)
				{
					property_value = value;
				}
			}
			else
			{
				T& value = std::get<T>(property_value);
				result = serializer.serialize(property_name, value);
			}

			return true;
		}

		template <typename... Types>
		Vadon::Utilities::Serializer::Result serialize_trivial_property_fold(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
		{
			Vadon::Utilities::Serializer::Result result;
			const bool fold_result = (serialize_trivial_property<Types>(serializer, property_name, property_value, result, data_type) || ...);
			if (fold_result == false)
			{
				result = Vadon::Utilities::Serializer::Result::NOT_IMPLEMENTED;
			}

			return result;
		}

		Vadon::Utilities::Serializer::Result process_trivial_property(Vadon::Utilities::Serializer& serializer, std::string_view property_name, Vadon::Utilities::Variant& property_value, Vadon::Utilities::ErasedDataTypeID data_type)
		{
			return serialize_trivial_property_fold<int, float, bool, std::string, Utilities::Vector2, Utilities::Vector2i, Utilities::Vector3, Utilities::UUID>(serializer, property_name, property_value, data_type);
		}

		bool serialize_component(Vadon::Scene::ResourceSystem& resource_system, Vadon::Utilities::Serializer& serializer, size_t index, SceneData::ComponentData& component_data)
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
				// FIXME: this forces us to iterate over all properties, instead of just reading the ones present in the object
				// Need to iterate over the K/V pairs instead
				// FIXME2: we also need to make sure we discard properties not present in the object (i.e if the properties changed)
				const Vadon::Utilities::PropertyInfoList component_properties = Vadon::Utilities::TypeRegistry::get_type_properties(component_data.type_id);
				SceneData::ComponentData::Property property_data;
				for (const Vadon::Utilities::PropertyInfo& current_property : component_properties)
				{
					switch (current_property.data_type.type)
					{
					case ErasedDataType::TRIVIAL:
					{
						const SerializerResult result = process_trivial_property(serializer, current_property.name, property_data.value, current_property.data_type);
						if (result == SerializerResult::SUCCESSFUL)
						{
							property_data.name = current_property.name;
							property_data.data_type = current_property.data_type;
							component_data.properties.push_back(property_data);
						}
						else
						{
							// TODO: log error?
						}
					}
						break;
					case ErasedDataType::RESOURCE_HANDLE:
					{
						ResourceHandle temp_resource_handle;
						if (resource_system.serialize_resource_property(serializer, current_property.name, temp_resource_handle) == true)
						{
							property_data.name = current_property.name;
							property_data.data_type = current_property.data_type;
							property_data.value = temp_resource_handle;
							component_data.properties.push_back(property_data);
						}
						else
						{
							// TODO: log error?
						}
					}
						break;
					}
				}
			}
			else
			{
				for (SceneData::ComponentData::Property& current_property : component_data.properties)
				{
					switch (current_property.data_type.type)
					{
					case ErasedDataType::TRIVIAL:
					{
						const SerializerResult result = process_trivial_property(serializer, current_property.name, current_property.value, current_property.data_type);
						if (result != SerializerResult::SUCCESSFUL)
						{
							// TODO: log error?
						}
					}
					break;
					case ErasedDataType::RESOURCE_HANDLE:
					{
						ResourceHandle temp_resource_handle = std::get<ResourceHandle>(current_property.value);
						if (resource_system.serialize_resource_property(serializer, current_property.name, temp_resource_handle) == false)
						{
							// TODO: log error?
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

		bool serialize_entity(Vadon::Scene::ResourceSystem& resource_system, Vadon::Utilities::Serializer& serializer, size_t index, SceneData::EntityData& entity_data)
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
				resource_system.serialize_resource_property(serializer, "scene", entity_data.scene);
			}

			if(serializer.open_array("components") != SerializerResult::SUCCESSFUL)
			{
				Vadon::Core::Logger::log_error(c_component_array_error_message);
				return false;
			}

			const size_t component_count = serializer.is_reading() ? serializer.get_array_size() : entity_data.components.size();
			for (size_t current_component_index = 0; current_component_index < component_count; ++current_component_index)
			{
				SceneData::ComponentData& current_component_data = serializer.is_reading() ? entity_data.components.emplace_back() : entity_data.components[current_component_index];
				if (serialize_component(resource_system, serializer, current_component_index, current_component_data) == false)
				{
					return false;
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
	}

	void SceneData::register_scene_type_info()
	{
		Vadon::Scene::ResourceRegistry::register_resource_type<Scene, ResourceBase>();

		Vadon::ECS::ComponentRegistry::register_component_type<SceneComponent>();
	}

	SceneHandle SceneSystem::create_scene()
	{
		SceneHandle new_scene_handle;
		new_scene_handle.from_resource_handle(m_resource_system.create_resource<Scene>());

		if (new_scene_handle.is_valid() == false)
		{
			log_error("Scene system: failed to create scene!\n");
			return new_scene_handle;
		}

		if (internal_add_scene(new_scene_handle) == false)
		{
			log_error("Scene system: failed to add new scene!\n");
			// TODO2: remove resource!
			return SceneHandle();
		}

		return new_scene_handle;
	}

	SceneHandle SceneSystem::find_scene(ResourceID scene_id) const
	{
		const ResourceHandle resource_handle = m_resource_system.find_resource(scene_id);
		if (resource_handle.is_valid() == false)
		{
			return SceneHandle();
		}

		if (m_resource_system.get_resource_info(resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Scene>())
		{
			log_error("Scene system: resource ID does not correspond to scene resource!\n");
			return SceneHandle();
		}

		return SceneHandle().from_resource_handle(resource_handle);
	}

	bool SceneSystem::package_scene_data(SceneHandle scene_handle, ECS::World& ecs_world, ECS::EntityHandle root_entity)
	{
		SceneData* scene_data = get_scene_data(scene_handle);
		if (scene_data == nullptr)
		{
			log_error("Scene system error: attempting to package scene, but no scene data is present!\n");
			return false;
		}

		SceneData temp_scene_data;
		{
			// Track dependency stack (ensures we cannot save an invalid scene)
			std::vector<SceneHandle> dependency_stack;
			dependency_stack.push_back(scene_handle);

			if (parse_scene_entity(ecs_world, root_entity, -1, temp_scene_data, dependency_stack) == false)
			{
				return false;
			}
		}
		scene_data->swap(temp_scene_data);

		return true;
	}

	bool SceneSystem::save_scene(SceneHandle scene_handle)
	{
		SceneData* scene_data = find_scene_data(scene_handle);
		if (scene_data == nullptr)
		{
			log_error("Scene system error: attempting to save scene, but no scene data is present!\n");
			return false;
		}

		const ResourceInfo scene_info = m_resource_system.get_resource_info(scene_handle);
		if (scene_info.path.is_valid() == false)
		{
			log_error("Scene system: cannot save scene with no path!\n");
			return false;
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Core::FileSystem::RawFileDataBuffer scene_file_buffer;
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(scene_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::WRITE);

		if (serializer->initialize() == false)
		{
			log_error("Scene system: failed to initialize serializer while saving scene!\n");
			return false;
		}

		if (serialize_scene(*serializer, scene_handle, *scene_data) == false)
		{
			log_error("Scene system: failed to serialize while saving scene!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			log_error("Scene system: failed to finalize serializer while saving scene!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		if (file_system.save_file(scene_info.path, scene_file_buffer) == false)
		{
			log_error("Scene system: failed to save scene data to file!\n");
			return false;
		}

		return true;
	}

	bool SceneSystem::load_scene(SceneHandle scene_handle)
	{
		SceneData* scene_data = find_scene_data(scene_handle);
		if (scene_data != nullptr)
		{
			// Assume we already loaded the scene if we have the scene data object
			return true;
		}

		const ResourceInfo scene_info = m_resource_system.get_resource_info(scene_handle);
		if (scene_info.path.is_valid() == false)
		{
			log_error("Scene system: cannot load scene without a valid path!\n");
			return false;
		}

		Vadon::Core::FileSystem& file_system = m_engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystem::RawFileDataBuffer scene_file_buffer;
		if (file_system.load_file(scene_info.path, scene_file_buffer) == false)
		{
			log_error("Scene system: failed to load scene file!\n");
			return false;
		}

		// FIXME: support binary file serialization!
		// Solution: have file system create the appropriate serializer!
		Vadon::Utilities::Serializer::Instance serializer = Vadon::Utilities::Serializer::create_serializer(scene_file_buffer, Vadon::Utilities::Serializer::Type::JSON, Vadon::Utilities::Serializer::Mode::READ);

		if (serializer->initialize() == false)
		{
			log_error("Scene system: failed to initialize serializer while loading scene!\n");
			return false;
		}

		// Create temporary scene data so we can discard if something goes wrong
		SceneData temp_scene_data;
		if (serialize_scene(*serializer, scene_handle, temp_scene_data) == false)
		{
			log_error("Scene system: failed to deserialize scene data!\n");
			return false;
		}

		if (serializer->finalize() == false)
		{
			log_error("Scene system: failed to finalize serializer after loading scene data!\n");
			return false;
		}

		SceneData* loaded_scene_data = get_scene_data(scene_handle);
		if (loaded_scene_data == nullptr)
		{
			log_error("Scene system: failed to create scene object while loading scene!\n");
			return false;
		}

		loaded_scene_data->swap(temp_scene_data);

		return true;
	}

	ECS::EntityHandle SceneSystem::instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene)
	{
		// FIXME: should we perform circular dependency validation?
		// In principle we cannot have an invalid scene by this stage

		// Make sure the scene is loaded
		if (load_scene(scene_handle) == false)
		{
			log_error("Scene system: failed to load scene, unable to instantiate!\n");
			return ECS::EntityHandle();
		}

		const SceneData* scene_data = find_scene_data(scene_handle);
		if (scene_data == nullptr)
		{
			log_error("Scene system error: attempting to instantiate scene, but no scene data is present!\n");
			return ECS::EntityHandle();
		}

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

	bool SceneSystem::is_scene_dependent(SceneHandle base_scene_handle, SceneHandle dependent_scene_handle)
	{
		std::vector<SceneHandle> dependency_stack;
		dependency_stack.push_back(base_scene_handle);
		if (internal_is_scene_dependent(dependent_scene_handle, dependency_stack) == true)
		{
			return true;
		}

		return false;
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

		m_resource_system.register_resource_file_extension(".vdsc");

		SceneData::register_scene_type_info();
		log_message("Scene System initialized!\n");
		return true;
	}

	void SceneSystem::shutdown()
	{
		log_message("Shutting down Scene System\n");
		m_resource_system.shutdown();
		m_scene_data_lookup.clear();
		log_message("Scene System shut down!\n");
	}

	bool SceneSystem::internal_add_scene(SceneHandle scene_handle)
	{
		if (find_scene_data(scene_handle) != nullptr)
		{
			log_error("Scene system: scene already added!\n");
			return false;
		}

		m_scene_data_lookup.emplace(scene_handle.to_uint(), SceneData{});
		return true;
	}

	bool SceneSystem::serialize_scene(Vadon::Utilities::Serializer& serializer, SceneHandle scene_handle, SceneData& scene_data)
	{
		// First serialize as resource
		if (m_resource_system.serialize_resource(serializer, scene_handle) == false)
		{
			log_error("Scene system: failed to serialize resource data for scene!\n");
			return false;
		}

		using SerializerResult = Vadon::Utilities::Serializer::Result;
		constexpr const char* c_scene_obj_error_log = "Scene system: unable to serialize scene object!\n";

		if (serializer.open_object("scene_data") != SerializerResult::SUCCESSFUL)
		{
			log_error(c_scene_obj_error_log);
			return false;
		}

		constexpr const char* c_entity_array_error_log = "Scene system: unable to serialize component object!\n";

		if (serializer.open_array("entities") != SerializerResult::SUCCESSFUL)
		{
			log_error(c_entity_array_error_log);
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
				if (serialize_entity(m_resource_system, serializer, current_entity_index, current_entity_data) == false)
				{
					return false;
				}
			}
		}
		else
		{
			for (size_t current_entity_index = 0; current_entity_index < scene_data.entities.size(); ++current_entity_index)
			{
				if (serialize_entity(m_resource_system, serializer, current_entity_index, scene_data.entities[current_entity_index]) == false)
				{
					return false;
				}
			}
		}

		if (serializer.close_array() != SerializerResult::SUCCESSFUL)
		{
			log_error(c_entity_array_error_log);
			return false;
		}

		if (serializer.close_object() != SerializerResult::SUCCESSFUL)
		{
			log_error(c_scene_obj_error_log);
			return false;
		}

		return true;
	}

	bool SceneSystem::parse_scene_entity(ECS::World& ecs_world, ECS::EntityHandle entity, int32_t parent_index, SceneData& scene_data, std::vector<SceneHandle>& dependency_stack)
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
				log_error("Scene system: Entity has Scene Component but no valid metadata!\n");
				return false;
			}

			// Make sure we don't have a circular dependency
			// FIXME: use LUT to make sure we don't check more than once?
			if (internal_is_scene_dependent(scene_comp->root_scene, dependency_stack) == true)
			{
				log_error("Scene system: parsed scene has circular dependency!\n");
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
				current_property_data.data_type = current_component_property.data_type;
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

	bool SceneSystem::internal_is_scene_dependent(SceneHandle dependent_scene_handle, std::vector<SceneHandle>& dependency_stack)
	{
		if (std::find(dependency_stack.begin(), dependency_stack.end(), dependent_scene_handle) != dependency_stack.end())
		{
			return true;
		}

		// Add to stack, recursively check entities if they might lead to a circular dependency.
		// If an entity references a scene which is already present in the dependency stack
		// that means we are trying to load a sub-scene which transitively loops back
		// to one of the earlier dependencies in the chain
		// In other words, we're testing to make sure we have a DAG
		dependency_stack.push_back(dependent_scene_handle);

		const SceneData* scene_data = find_scene_data(dependent_scene_handle);
		if (scene_data == nullptr)
		{
			// Load and retry
			if (load_scene(dependent_scene_handle) == false)
			{
				log_error("Scene system: failed to load scene during dependency check!\n");
				return true;
			}
			scene_data = find_scene_data(dependent_scene_handle);

			if (scene_data == nullptr)
			{
				log_error("Scene system: performing dependency check on scene, but no scene data is present!\n");
				return true;
			}
		}

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

	const SceneData* SceneSystem::find_scene_data(SceneHandle scene_handle) const
	{
		auto scene_data_it = m_scene_data_lookup.find(scene_handle.to_uint());
		if (scene_data_it == m_scene_data_lookup.end())
		{
			return nullptr;
		}

		return &scene_data_it->second;
	}

	SceneData* SceneSystem::get_scene_data(SceneHandle scene_handle)
	{
		SceneData* scene_data = find_scene_data(scene_handle);
		if (scene_data != nullptr)
		{
			return scene_data;
		}

		if (m_resource_system.has_resource(scene_handle) == false)
		{
			log_error("Scene system: scene resource not found!\n");
			return nullptr;
		}

		if (m_resource_system.get_resource_info(scene_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Scene>())
		{
			log_error("Scene system: scene resource type mismatch!\n");
			return nullptr;
		}

		if (internal_add_scene(scene_handle) == false)
		{
			log_error("Scene system: failed to add scene!\n");
			return nullptr;
		}

		return find_scene_data(scene_handle);
	}
}