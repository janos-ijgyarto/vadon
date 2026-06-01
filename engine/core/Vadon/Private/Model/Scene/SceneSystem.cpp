#include <Vadon/Private/Model/Scene/SceneSystem.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Model/Resource/Registry.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

#include <format>

namespace 
{
	void clear_entity_data(Vadon::Private::Model::EntityData& entity)
	{
		// We need to destroy each component in each entity, since they were heap-allocated
		// FIXME: use refcounting and/or some kind of allocator strategy to make this more robust!
		for (Vadon::Private::Model::ComponentData& current_component : entity.components)
		{
			Vadon::Utilities::TypeRegistry::destroy_object(current_component);
		}

		entity.components.clear();
	}
}

namespace Vadon::Model
{
	Scene::~Scene()
	{
		for (Vadon::Private::Model::EntityData& current_entity : entities)
		{
			clear_entity_data(current_entity);
		}
	}
}

namespace Vadon::Private::Model
{
	SceneHandle SceneSystem::create_scene()
	{
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
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
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
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
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
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

	ECS::EntityHandle SceneSystem::instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, bool is_sub_scene)
	{
		// TODO: circular dependency check?
		const Scene* scene = get_scene(scene_handle);
		std::unordered_map<::Vadon::Foundation::UUID, Vadon::ECS::EntityHandle> entity_lookup;

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		constexpr auto clean_up_scene = +[](ECS::World& world, const Scene* scene_ptr, std::unordered_map<::Vadon::Foundation::UUID, Vadon::ECS::EntityHandle>& lookup)
			{
				if (lookup.empty() == false)
				{
					// Clean up from root
					const Vadon::ECS::EntityHandle root_entity = lookup[scene_ptr->entities.front().id];
					world.remove_entity(root_entity);
				}
			};

		for (const EntityData& current_entity_data : scene->entities)
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

					clean_up_scene(ecs_world, scene, entity_lookup);
					return Vadon::ECS::EntityHandle();
				}
				current_entity = instantiate_scene(sub_scene_handle, ecs_world, true);
				if (current_entity.is_valid() == false)
				{
					clean_up_scene(ecs_world, scene, entity_lookup);
					return Vadon::ECS::EntityHandle();
				}
			}

			for (const ComponentData& current_component_data : current_entity_data.components)
			{
				Vadon::ECS::ComponentHandle current_component = current_entity_data.scene.is_valid() == false
					? component_manager.add_component(current_entity, current_component_data.type)
					: component_manager.get_component(current_entity, current_component_data.type);

				if (current_component.is_valid() == false)
				{
					// TODO: error?
					continue;
				}

				// FIXME: we're copying components per-propert
				// We should instead pass the object as a whole and "clone" it
				const Vadon::Utilities::PropertyList component_properties = Vadon::Utilities::TypeRegistry::get_properties(current_component_data.data, current_component_data.type);
				for (const Vadon::Utilities::Property& current_property_data : component_properties)
				{
					Vadon::Utilities::TypeRegistry::set_property(current_component.get_raw(), current_component_data.type, current_property_data.info.id, current_property_data.value);
				}
			}

			entity_lookup.insert(std::make_pair(current_entity_data.id, current_entity));

			if (current_entity_data.has_parent() == true)
			{
				// TODO: set parent!
				Vadon::ECS::EntityHandle parent_entity = entity_lookup[current_entity_data.parent];
				entity_manager.add_child_entity(parent_entity, current_entity);
			}
		}

		const Vadon::ECS::EntityHandle root_entity = entity_lookup[scene->entities.front().id];
		if (is_sub_scene == true)
		{
			// Add scene component
			Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
			const SceneID scene_id = SceneID::from_resource_id(resource_system.get_resource_info(scene_handle).id);

			auto root_scene_component = component_manager.add_component<SceneComponent>(root_entity);
			root_scene_component->root_scene = scene_id;

			// Add scene component to each child, indicates that these were instantiated from the scene
			for (Vadon::ECS::EntityHandle current_child : entity_manager.get_children(root_entity))
			{
				if (component_manager.has_component<SceneComponent>(current_child) == false)
				{
					component_manager.add_component<SceneComponent>(current_child);
				}

				auto child_scene_component = component_manager.get_component<SceneComponent>(current_child);
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
		: Vadon::Model::SceneSystem(core)
		, m_animation_system(core)
	{}

	void SceneSystem::register_types()
	{
		Vadon::Model::ResourceRegistry::register_resource_type<Vadon::Model::Scene, Resource>();
		Vadon::Utilities::TypeRegistry::register_type<EntityData>();

		Vadon::Utilities::TypeRegistry::add_property<EntityData>(VADON_GET_MEMBER_UUID(EntityData, id), Vadon::Utilities::MemberVariableBind<&EntityData::id>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<EntityData>(VADON_GET_MEMBER_UUID(EntityData, parent), Vadon::Utilities::MemberVariableBind<&EntityData::parent>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<EntityData>(VADON_GET_MEMBER_UUID(EntityData, scene), Vadon::Utilities::MemberVariableBind<&EntityData::scene>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<EntityData>(VADON_GET_MEMBER_UUID(EntityData, components), Vadon::Utilities::MemberVariableBind<&EntityData::components>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<EntityData>(VADON_GET_MEMBER_UUID(EntityData, name), Vadon::Utilities::MemberVariableBind<&EntityData::name>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeRegistry::add_property<Vadon::Model::Scene>(VADON_GET_MEMBER_UUID(Vadon::Model::Scene, entities), Vadon::Utilities::MemberVariableBind<&Vadon::Model::Scene::entities>().bind_member_getter().bind_member_setter());

		Vadon::ECS::ComponentRegistry::register_component_type<SceneComponent>();

		AnimationSystem::register_types();
	}

	void SceneSystem::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		constexpr ::Vadon::Foundation::UUID c_data_object_uuid = Vadon::Utilities::string_to_uuid(::Vadon::Foundation::DataObjectSchema::c_type_uuid);

		Vadon::Utilities::TypeMetadata entitydata_metadata(metadata_registry, VADON_GET_TYPE_UUID(EntityData));
		entitydata_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Private::Scene::EntityData")
			.add_property(EntityData::c_id_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "ID")
				.commit_property()
			.add_property(EntityData::c_parent_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Parent")
				.commit_property()
			.add_property(EntityData::c_scene_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Scene")
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(Vadon::Model::Scene))
				.commit_property()
			.add_property(EntityData::c_components_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Components")
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::ARRAY_TYPE, Vadon::Utilities::uuid_to_base64_string(c_data_object_uuid).c_str())
				.commit_property()
			.add_property(EntityData::c_name_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Name");

		Vadon::Utilities::TypeMetadata scene_metadata(metadata_registry, VADON_GET_TYPE_UUID(Vadon::Model::Scene));
		scene_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::Scene");

		scene_metadata.add_property(Vadon::Utilities::Property::property_schema_to_uuid(::Vadon::Foundation::SceneSchema::c_entities_property))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Entities")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::ARRAY_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(EntityData));

		AnimationSystem::register_type_metadata(metadata_registry);
	}

	bool SceneSystem::initialize()
	{
		log_message("Initializing Scene System\n");

		if (m_animation_system.initialize() == false)
		{
			return false;
		}

		log_message("Scene System initialized!\n");
		return true;
	}

	void SceneSystem::shutdown()
	{
		log_message("Shutting down Scene System\n");
		m_animation_system.shutdown();
		log_message("Scene System shut down!\n");
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
		for (const EntityData& current_entity_data : scene->entities)
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
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		return resource_system.get_resource<Scene>(scene_handle);
	}
}