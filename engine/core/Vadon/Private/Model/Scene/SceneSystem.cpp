#include <Vadon/Private/Model/Scene/SceneSystem.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Model/Resource/Registry.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

#include <Vadon/Foundation/TypeInfo/Object.hpp>

#include <format>

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

	ECS::EntityHandle SceneSystem::instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world)
	{
		InstantiatedSceneResult result = internal_instantiate_scene(scene_handle, ecs_world, SceneID{});
		return result.root_entity;
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
		Vadon::Utilities::TypeRegistry::add_property<Vadon::Model::Scene>(VADON_GET_MEMBER_UUID(Vadon::Model::Scene, base_scene), Vadon::Utilities::MemberVariableBind<&Vadon::Model::Scene::base_scene>().bind_member_getter().bind_member_setter());

		Vadon::ECS::ComponentRegistry::register_component_type<SceneComponent>();

		AnimationSystem::register_types();
	}

	void SceneSystem::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::Utilities::TypeMetadata<EntityData> entitydata_metadata(metadata_registry);
		entitydata_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Private::Model::EntityData")
			.add_property(EntityData::c_id_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "ID")
				.commit_property()
			.add_property(EntityData::c_parent_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Parent")
				.commit_property()
			.add_property(EntityData::c_scene_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Scene")
				.commit_property()
			.add_property(EntityData::c_components_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Components")
				.commit_property()
			.add_property(EntityData::c_name_member_id)
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Name");

		Vadon::Utilities::TypeMetadata<Vadon::Model::Scene> scene_metadata(metadata_registry);
		scene_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::Scene");

		scene_metadata.add_property(VADON_GET_MEMBER_UUID(Scene, entities))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Entities")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
			.commit_property()
		.add_property(VADON_GET_MEMBER_UUID(Scene, base_scene))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Base Scene")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN));

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

	SceneSystem::InstantiatedSceneResult SceneSystem::internal_instantiate_scene(SceneHandle scene_handle, ECS::World& ecs_world, const SceneID& parent_scene_id, const SceneID& derived_scene_id)
	{
		InstantiatedSceneResult result;

		// TODO: circular dependency check?
		const Scene* scene = get_scene(scene_handle);

		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		const SceneID scene_id = SceneID::from_resource_id(resource_system.get_resource_info(scene_handle).id);

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		constexpr auto clean_up_scene = +[](ECS::World& world, InstantiatedSceneResult& scene_result)
			{
				// Clean up from root
				if (scene_result.root_entity.is_valid() == true)
				{
					world.remove_entity(scene_result.root_entity);
					scene_result.root_entity.invalidate();
				}
				scene_result.entity_lookup.clear();
			};

		if (scene->base_scene.is_valid() == true)
		{
			// Load the base scene first
			const SceneHandle base_scene_handle = load_scene(scene->base_scene);
			if (base_scene_handle.is_valid() == false)
			{
				// TODO: log scene ID!
				// TODO2: should we abort, or just skip the entity that failed to load?
				log_error("Scene system: failed to load base scene while instantiating!\n");
				return result;
			}

			// Pass in the derived scene ID so the components are set accordingly
			// NOTE: when we instantiate a "grandchild", we keep using its scene ID even for the "grandparent" entities
			result = internal_instantiate_scene(base_scene_handle, ecs_world, parent_scene_id, derived_scene_id.is_valid() ? derived_scene_id : scene_id);
			if (result.root_entity.is_valid() == false)
			{
				log_error("Scene system: failed to instantiate base scene!\n");
				return result;
			}
		}
		else
		{
			VADON_ASSERT(scene->entities.empty() == false, "Scene must have at least one root entity!");
		}

		constexpr auto load_component_properties = +[](const ComponentData& component_data, Vadon::ECS::ComponentHandle& component_handle)
			{
				for (const auto& current_property_pair : component_data.get_properties().data)
				{
					Utilities::PropertyUUID property_id;
					if (Utilities::uuid_from_base64_string(current_property_pair.first, property_id) == false)
					{
						// TODO: log warning/error for stale data?
						continue;
					}
					Vadon::Utilities::TypeRegistry::set_property(component_handle.get_raw(), component_data.get_type_id(), property_id, current_property_pair.second);
				}
			};

		for (const EntityData& current_entity_data : scene->entities)
		{
			if (scene->base_scene.is_valid() == true)
			{
				auto entity_lookup_it = result.entity_lookup.find(current_entity_data.id);
				if (entity_lookup_it != result.entity_lookup.end())
				{
					const Vadon::ECS::EntityHandle current_entity = entity_lookup_it->second;
					for (const ComponentData& current_component_data : current_entity_data.components)
					{
						Vadon::ECS::ComponentHandle current_component = component_manager.get_component(current_entity, current_component_data.get_type_id());
						if (current_component.is_valid() == false)
						{
							// Current component is added by the derived scene, so we add it
							VADON_ASSERT(current_entity_data.scene.is_valid() == false, "Cannot add components to sub-scenes!");
							current_component = component_manager.add_component(current_entity, current_component_data.get_type_id());
						}

						load_component_properties(current_component_data, current_component);
					}
					continue;
				}
			}

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

					clean_up_scene(ecs_world, result);
					return result;
				}

				InstantiatedSceneResult sub_scene_result = internal_instantiate_scene(sub_scene_handle, ecs_world, derived_scene_id.is_valid() ? derived_scene_id : scene_id);
				if (sub_scene_result.is_valid() == false)
				{
					clean_up_scene(ecs_world, result);
					return result;
				}

				current_entity = sub_scene_result.root_entity;
			}

			// Load the component data
			// NOTE: in case of instantiated sub-scene, this will override values set in the owner scene
			for (const ComponentData& current_component_data : current_entity_data.components)
			{
				Vadon::ECS::ComponentHandle current_component = current_entity_data.scene.is_valid() == false
					? component_manager.add_component(current_entity, current_component_data.get_type_id())
					: component_manager.get_component(current_entity, current_component_data.get_type_id());

				if (current_component.is_valid() == false)
				{
					// TODO: in case it's a sub-scene, notify that we have stale component data
					// Otherwise throw error for invalid data
					continue;
				}

				load_component_properties(current_component_data, current_component);
			}

			// Add to the lookup
			VADON_ASSERT(result.entity_lookup.find(current_entity_data.id) == result.entity_lookup.end(), "Entity already present in lookup!");
			result.entity_lookup.insert(std::make_pair(current_entity_data.id, current_entity));

			if (current_entity_data.has_parent() == true)
			{
				auto parent_it = result.entity_lookup.find(current_entity_data.parent);
				VADON_ASSERT(parent_it != result.entity_lookup.end(), "Cannot find entity parent");

				Vadon::ECS::EntityHandle parent_entity = parent_it->second;
				entity_manager.add_child_entity(parent_entity, current_entity);
			}
			else
			{
				// No parent, so this must be the root entity
				VADON_ASSERT(result.root_entity.is_valid() == false, "Scene cannot have multiple roots!");
				result.root_entity = current_entity;
			}

			// Add scene component
			if (component_manager.has_component<SceneComponent>(current_entity) == false)
			{
				component_manager.add_component<SceneComponent>(current_entity);
			}
			auto scene_component = component_manager.get_component<SceneComponent>(current_entity);
			if (scene_component->scene_info.scene_id.is_valid() == true)
			{
				// Entity is also the root of a sub-scene
				// Data is already set, we just need to set its ID from the parent scene
				scene_component->parent_scene_info.entity_id = current_entity_data.id;
			}
			else
			{
				// Entity is defined within this scene, set the Scene Component contents
				scene_component->scene_info.scene_id = derived_scene_id.is_valid() ? derived_scene_id : scene_id;
				scene_component->scene_info.entity_id = current_entity_data.id;

				if (parent_scene_id.is_valid() == true)
				{
					// Entity is created as a sub-scene in an owning scene
					scene_component->parent_scene_info.scene_id = parent_scene_id;
				}
			}
		}

		VADON_ASSERT(result.root_entity.is_valid() == true, "No root entity was set!");
		return result;
	}

	const Scene* SceneSystem::get_scene(SceneHandle scene_handle) const
	{
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		return resource_system.get_resource<Scene>(scene_handle);
	}
}