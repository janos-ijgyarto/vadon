#include <VadonDemo/Model/Weapon/System.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

namespace VadonDemo::Model
{
	WeaponSystem::WeaponSystem(Core::Core& core)
		: m_core(core)
	{

	}

	void WeaponSystem::register_types()
	{
		WeaponDefinition::register_resource();

		WeaponComponent::register_component();

		ProjectileComponent::register_component();
		ProjectileHomingComponent::register_component();
		ProjectileAOEComponent::register_component();

		Vadon::ECS::ComponentRegistry::register_tag_type<ProjectileExplosionTag>();
	}

	void WeaponSystem::update(Vadon::ECS::World& ecs_world, float delta_time)
	{
		update_weapons(ecs_world, delta_time);
		update_projectiles(ecs_world, delta_time);
	}

	void WeaponSystem::update_weapons(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();

		auto weapon_query = component_manager.run_component_query<WeaponComponent&>();
		for (auto weapon_it = weapon_query.get_iterator(); weapon_it.is_valid() == true; weapon_it.next())
		{
			auto weapon_tuple = weapon_it.get_tuple();
			WeaponComponent& current_weapon = std::get<WeaponComponent&>(weapon_tuple);

			if (current_weapon.firing_timer > 0.0f)
			{
				current_weapon.firing_timer -= delta_time;
			}

			if (current_weapon.firing_timer <= 0.0f)
			{
				// Reset firing timer
				Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

				const WeaponDefHandle weapon_def_handle = resource_system.load_resource(current_weapon.definition);
				const WeaponDefinition* weapon_def = resource_system.get_resource(weapon_def_handle);
				current_weapon.firing_timer += 60.0f / std::max(weapon_def->rate_of_fire, 0.001f);

				// IMPORTANT: this step will modify component containers
				// Must refresh the iterator to avoid problems after this point
				create_projectile(ecs_world, weapon_it.get_entity(), weapon_def);
			}
		}
	}

	void WeaponSystem::update_projectiles(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// FIXME: this is just a hacky implementation
		// Long-term there needs to be a more modular approach to managing lots of projectiles and their effects
		{
			auto explosion_query = component_manager.run_component_query<ProjectileExplosionTag&>();
			for (auto explosion_it = explosion_query.get_iterator(); explosion_it.is_valid() == true; explosion_it.next())
			{
				// Explosion should be removed by next frame
				// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
				// Whenever we spawn a projectile again, we can just "resurrect" a projectile that was already used
				component_manager.set_entity_tag<DestroyEntityTag>(explosion_it.get_entity(), true);
			}
		}

		// Check if any projectiles timed out
		{
			auto projectile_query = component_manager.run_component_query<ProjectileComponent&, ProjectileAOEComponent*, ProjectileExplosionTag*>();
			for (auto projectile_it = projectile_query.get_iterator(); projectile_it.is_valid() == true; projectile_it.next())
			{
				auto projectile_tuple = projectile_it.get_tuple();

				const ProjectileExplosionTag* current_explosion = std::get<ProjectileExplosionTag*>(projectile_tuple);
				if (current_explosion != nullptr)
				{
					continue;
				}

				ProjectileComponent* current_projectile = &std::get<ProjectileComponent&>(projectile_tuple);
				current_projectile->remaining_lifetime -= delta_time;

				if (current_projectile->remaining_lifetime <= 0.0f)
				{
					// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
					// Whenever we spawn a projectile again, we can just "resurrect" a projectile that was already used
					component_manager.set_entity_tag<DestroyEntityTag>(projectile_it.get_entity(), true);

					// If projectile has AOE, spawn an explosion
					const ProjectileAOEComponent* aoe_component = std::get<ProjectileAOEComponent*>(projectile_tuple);
					if (aoe_component != nullptr)
					{
						// FIXME: this is all really janky
						// Need a better system to ensure that adding/removing Entities or Components
						// does not invalidate iterators/references/etc. until we reach a sync point!
						Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
						Vadon::ECS::EntityHandle explosion_entity = entity_manager.create_entity();

						{
							Transform2D& explosion_transform = component_manager.add_component<Transform2D>(explosion_entity);

							Transform2D* original_transform = component_manager.get_component<Transform2D>(projectile_it.get_entity());
							if (original_transform != nullptr)
							{
								explosion_transform = *original_transform;
							}
							VADON_ASSERT(explosion_transform.scale > 0.0f, "Something is wrong!");
						}
						{
							ProjectileComponent& explosion_projectile = component_manager.add_component<ProjectileComponent>(explosion_entity);

							// FIXME: need to do this because the iterator gets invalidated!
							projectile_it.refresh();
							current_projectile = &std::get<ProjectileComponent&>(projectile_it.get_tuple());

							explosion_projectile.damage = current_projectile->damage;
							explosion_projectile.knockback = current_projectile->knockback;
							// TODO: copy any other stats?
						}
						{
							Collision& explosion_collision = component_manager.add_component<Collision>(explosion_entity);
							explosion_collision.radius = aoe_component->radius;
						}

						// Set the explosion tag
						component_manager.set_entity_tag<ProjectileExplosionTag>(explosion_entity, true);

						// Add under root entity
						// TODO: create root for projectiles?
						Vadon::ECS::EntityHandle root_entity = Model::get_root_entity(ecs_world);
						entity_manager.set_entity_parent(explosion_entity, root_entity);

						// Dispatch event
						m_core.entity_added(ecs_world, explosion_entity);
					}
				}
			}
		}
	}

	void WeaponSystem::create_projectile(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle weapon_entity, const WeaponDefinition* weapon_def)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();

		// Spawn projectile
		// FIXME: at the moment we make an entity for every projectile
		// Could use a special "pool" Entity that contains projectile instances
		const Vadon::Scene::SceneHandle spawned_projectile_scene = scene_system.load_scene(weapon_def->projectile_prefab);
		const Vadon::ECS::EntityHandle spawned_projectile = scene_system.instantiate_scene(spawned_projectile_scene, ecs_world);

		// FIXME: make a parent entity for projectiles?
		// Could be the projectile subsystem
		ecs_world.get_entity_manager().add_child_entity(Model::get_root_entity(ecs_world), spawned_projectile);

		// Get the weapon that fired the projectile
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto weapon_tuple = component_manager.get_component_tuple<Transform2D, WeaponComponent>(weapon_entity);

		auto projectile_tuple = component_manager.get_component_tuple<Transform2D, Velocity2D, ProjectileComponent>(spawned_projectile);

		// Set the initial position
		Transform2D* weapon_transform = std::get<Transform2D*>(weapon_tuple);
		Transform2D* projectile_transform = std::get<Transform2D*>(projectile_tuple);

		VADON_ASSERT((weapon_transform != nullptr) && (projectile_transform != nullptr), "Invalid component setup!");
		projectile_transform->position = weapon_transform->position;

		// Set velocity and lifetime
		Velocity2D* projectile_velocity = std::get<Velocity2D*>(projectile_tuple);
		ProjectileComponent* projectile_component = std::get<ProjectileComponent*>(projectile_tuple);

		projectile_component->remaining_lifetime = projectile_component->range / projectile_velocity->top_speed;

		Vadon::Math::Vector2 initial_direction = Vadon::Math::Vector2_One;

		ProjectileHomingComponent* homing_component = component_manager.get_component<ProjectileHomingComponent>(spawned_projectile);
		if (homing_component != nullptr)
		{
			// Find nearest target
			// TODO: change which targets we look for depending on who is firing the weapon
			Vadon::ECS::EntityHandle selected_target;
			float min_distance = std::numeric_limits<float>::max();
			auto enemy_query = component_manager.run_component_query<Enemy&, Transform2D&>();
			for (auto enemy_it = enemy_query.get_iterator(); enemy_it.is_valid() == true; enemy_it.next())
			{
				auto enemy_tuple = enemy_it.get_tuple();
				const Transform2D& enemy_transform = std::get<Transform2D&>(enemy_tuple);

				const float distance_sq = Vadon::Math::Vector::distance_squared(weapon_transform->position, enemy_transform.position);
				if (distance_sq < min_distance)
				{
					selected_target = enemy_it.get_entity();
					min_distance = distance_sq;
				}
			}

			homing_component->target_entity = selected_target;
			if ((homing_component->aimed_on_launch == true) && (selected_target.is_valid() == true))
			{
				const Transform2D* target_transform = component_manager.get_component<Transform2D>(homing_component->target_entity);
				initial_direction = Vadon::Math::Vector::normalize(target_transform->position - projectile_transform->position);
			}
			else if (const Player* player_component = component_manager.get_component<Player>(weapon_entity))
			{
				// Choose the last direction the player moved in
				initial_direction = player_component->last_move_dir;
			}
		}
		else
		{
			if (const Player* player_component = component_manager.get_component<Player>(weapon_entity))
			{
				// Choose the last direction the player moved in
				initial_direction = player_component->last_move_dir;
			}
		}

		// Set the velocity
		projectile_velocity->velocity = initial_direction * projectile_velocity->top_speed;

		// Dispatch event
		m_core.entity_added(ecs_world, spawned_projectile);
	}

	bool WeaponSystem::validate_weapon(const Player& player, const WeaponComponent& weapon_component)
	{
		if (player.starting_weapons.empty() == true)
		{
			// TODO: error!
			return false;
		}

		// TODO: validate starting weapon defs in player!
		if (weapon_component.definition.is_valid() == true)
		{
			// TODO: error!
			return false;
		}

		return true;
	}

	void WeaponSystem::projectile_collision_callback(Core::Core& /*core*/, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle projectile, Vadon::ECS::EntityHandle /*collider*/)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		ProjectileComponent* projectile_component = component_manager.get_component<ProjectileComponent>(projectile);
		VADON_ASSERT(projectile_component != nullptr, "Cannot find projectile component!");

		// Collision will make the projectile expire
		projectile_component->remaining_lifetime = 0.0f;
	}
}