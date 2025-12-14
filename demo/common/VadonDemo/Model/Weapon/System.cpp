#include <VadonDemo/Model/Weapon/System.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Enemy/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Math/Vector/Rotate.hpp>

namespace
{
	float angle_between_vectors(const Vadon::Math::Vector2& vec_a, const Vadon::Math::Vector2& vec_b)
	{
		const float dot = Vadon::Math::Vector::dot(vec_a, vec_b);
		const float det = vec_a.x * vec_b.y - vec_a.y * vec_b.x;
		return std::atan2f(det, dot);
	}
}

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
		WeaponVolleyComponent::register_component();

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

		auto weapon_query = component_manager.run_component_query<WeaponComponent&, WeaponVolleyComponent*>();
		for (auto weapon_it = weapon_query.get_iterator(); weapon_it.is_valid() == true; weapon_it.next())
		{
			auto current_weapon = weapon_it.get_component<WeaponComponent>();
			auto volley_component = weapon_it.get_component<WeaponVolleyComponent>();

			if (current_weapon->firing_timer > 0.0f)
			{
				current_weapon->firing_timer -= delta_time;
			}

			if (current_weapon->firing_timer <= 0.0f)
			{
				if (volley_component.is_valid() == true)
				{
					if (volley_component->fire_count == 0)
					{
						continue;
					}
				}

				// Reset firing timer
				Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

				const WeaponDefHandle weapon_def_handle = resource_system.load_resource(current_weapon->definition);
				const WeaponDefinition* weapon_def = resource_system.get_resource(weapon_def_handle);
				const float timer_reset_value = 60.0f / std::max(weapon_def->rate_of_fire, 0.001f);

				while (current_weapon->firing_timer <= 0.0f)
				{
					const float time_offset = std::abs(current_weapon->firing_timer);
					current_weapon->firing_timer += timer_reset_value;

					// IMPORTANT: this step will modify component containers
					// Must refresh the iterator to avoid problems after this point
					create_projectile(ecs_world, weapon_it.get_entity(), weapon_def, time_offset);

					if (volley_component.is_valid() == true)
					{
						volley_component->fire_count -= 1;
						if (volley_component->fire_count == 0)
						{
							break;
						}
					}
				}
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
			Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
			auto projectile_query = component_manager.run_component_query<ProjectileComponent&, ProjectileHomingComponent*, ProjectileAOEComponent*, ProjectileExplosionTag*>();
			for (auto projectile_it = projectile_query.get_iterator(); projectile_it.is_valid() == true; projectile_it.next())
			{
				const auto current_explosion = projectile_it.get_component<ProjectileExplosionTag>();
				if (current_explosion.is_valid() == true)
				{
					continue;
				}

				auto current_projectile = projectile_it.get_component<ProjectileComponent>();
				current_projectile->remaining_lifetime -= delta_time;

				if (current_projectile->remaining_lifetime <= 0.0f)
				{
					// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
					// Whenever we spawn a projectile again, we can just "resurrect" a projectile that was already used
					component_manager.set_entity_tag<DestroyEntityTag>(projectile_it.get_entity(), true);

					// If projectile has AOE, spawn an explosion
					const auto aoe_component = projectile_it.get_component<ProjectileAOEComponent>();
					if (aoe_component.is_valid() == true)
					{
						// FIXME: this is all really janky
						// Need a better system to ensure that adding/removing Entities or Components
						// does not invalidate iterators/references/etc. until we reach a sync point!
						Vadon::ECS::EntityHandle explosion_entity = entity_manager.create_entity();

						{
							auto explosion_transform = component_manager.add_component<Transform2D>(explosion_entity);

							const auto original_transform = component_manager.get_component<Transform2D>(projectile_it.get_entity());
							if (original_transform.is_valid() == true)
							{
								*explosion_transform = *original_transform;
							}
							VADON_ASSERT(explosion_transform->scale > 0.0f, "Something is wrong!");
						}
						{
							auto explosion_projectile = component_manager.add_component<ProjectileComponent>(explosion_entity);

							explosion_projectile->damage = current_projectile->damage;
							explosion_projectile->knockback = current_projectile->knockback;
							// TODO: copy any other stats?
						}
						{
							auto explosion_collision = component_manager.add_component<Collision>(explosion_entity);
							explosion_collision->radius = aoe_component->radius;
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

					// Projectile has expired
					continue;
				}

				auto homing_component = projectile_it.get_component<ProjectileHomingComponent>();
				if (homing_component.is_valid() == true)
				{
					// Make sure target is still valid
					if (homing_component->target_entity.is_valid() == true)
					{
						if (entity_manager.is_entity_valid(homing_component->target_entity) == false)
						{
							homing_component->target_entity.invalidate();
						}
					}

					if (homing_component->target_entity.is_valid() == true)
					{
						const auto target_transform = component_manager.get_component<Transform2D>(homing_component->target_entity);
						if (target_transform.is_valid() == true)
						{
							const auto projectile_transform = component_manager.get_component<Transform2D>(projectile_it.get_entity());
							auto projectile_velocity = component_manager.get_component<Velocity2D>(projectile_it.get_entity());

							const Vadon::Math::Vector2 direction = Vadon::Math::Vector::normalize(target_transform->position - projectile_transform->position);
							const Vadon::Math::Vector2 current_dir = Vadon::Math::Vector::normalize(projectile_velocity->velocity);

							const float turn_speed = std::max(homing_component->turn_speed, ProjectileHomingComponent::c_min_turn_speed);

							const float angle_diff = angle_between_vectors(current_dir, direction);
							float angle_change = delta_time * turn_speed * (std::signbit(angle_diff) ? -1.0f : 1.0f);
							if (std::abs(angle_change) > std::abs(angle_diff))
							{
								angle_change = angle_diff;
							}

							projectile_velocity->velocity = Vadon::Math::Vector::rotate(current_dir, angle_change) * projectile_velocity->top_speed;
						}
					}
				}
			}
		}
	}

	void WeaponSystem::create_projectile(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle weapon_entity, const WeaponDefinition* weapon_def, float time_offset)
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

		// Set the initial position
		auto weapon_transform = component_manager.get_component<Transform2D>(weapon_entity);
		auto projectile_transform = component_manager.get_component<Transform2D>(spawned_projectile);

		VADON_ASSERT((weapon_transform.is_valid() == true) && (projectile_transform.is_valid() == true), "Invalid component setup!");
		projectile_transform->position = weapon_transform->position;

		// Set velocity and lifetime
		auto projectile_velocity = component_manager.get_component<Velocity2D>(spawned_projectile);
		auto projectile_component = component_manager.get_component<ProjectileComponent>(spawned_projectile);

		projectile_component->remaining_lifetime = (projectile_component->range / projectile_velocity->top_speed) - time_offset;

		auto player_component = component_manager.get_component<Player>(weapon_entity);
		auto enemy_component = component_manager.get_component<EnemyBase>(weapon_entity);
		auto homing_component = component_manager.get_component<ProjectileHomingComponent>(spawned_projectile);

		// Find best target (logic depends on who is firing)
		Vadon::ECS::EntityHandle selected_target;

		if (player_component.is_valid() == true && homing_component.is_valid() == true)
		{
			float min_distance = std::numeric_limits<float>::max();
			auto enemy_query = component_manager.run_component_query<EnemyBase&, Transform2D&>();
			for (auto enemy_it = enemy_query.get_iterator(); enemy_it.is_valid() == true; enemy_it.next())
			{
				const auto enemy_transform = enemy_it.get_component<Transform2D>();

				const float distance_sq = Vadon::Math::Vector::distance_squared(weapon_transform->position, enemy_transform->position);
				if (distance_sq < min_distance)
				{
					selected_target = enemy_it.get_entity();
					min_distance = distance_sq;
				}
			}
		}
		else if (enemy_component.is_valid() == true)
		{
			projectile_component->enemy = true;

			auto player_query = component_manager.run_component_query<Player&, Velocity2D*, WeaponComponent*>();
			for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
			{
				if (selected_target.is_valid() == false)
				{
					selected_target = player_it.get_entity();
					break;
				}
			}
		}

		Vadon::Math::Vector2 target_direction = Vadon::Math::Vector2_One;
		if (selected_target.is_valid() == true)
		{
			const auto target_transform = component_manager.get_component<Transform2D>(selected_target);
			if (target_transform.is_valid() == true)
			{
				target_direction = Vadon::Math::Vector::normalize(target_transform->position - projectile_transform->position);
			}
		}

		Vadon::Math::Vector2 projectile_direction = Vadon::Math::Vector2_One;
		if (player_component.is_valid() == true)
		{
			// Choose the last direction the player moved in
			projectile_direction = player_component->last_move_dir;
		}
		else if (enemy_component.is_valid() == true)
		{
			// Enemies aim for the player
			projectile_direction = target_direction;
		}

		// Homing logic can override target
		if (homing_component.is_valid() == true)
		{
			homing_component->target_entity = selected_target;
			if (homing_component->aimed_on_launch == true)
			{
				projectile_direction = target_direction;
			}
		}

		// Set the velocity
		projectile_velocity->velocity = projectile_direction * projectile_velocity->top_speed;

		// Advance the projectile position by the time that passed relative to when it was fired
		projectile_transform->position = projectile_transform->position + (projectile_velocity->velocity * time_offset);

		// Dispatch event
		m_core.entity_added(ecs_world, spawned_projectile);
	}

	bool WeaponSystem::validate_weapon(const Vadon::ECS::TypedComponentHandle<Player>& player, const WeaponComponent& weapon_component)
	{
		if (player->starting_weapons.empty() == true)
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
		auto projectile_component = component_manager.get_component<ProjectileComponent>(projectile);
		VADON_ASSERT(projectile_component.is_valid() == true, "Cannot find projectile component!");

		// Collision will make the projectile expire
		projectile_component->remaining_lifetime = 0.0f;
	}

	void WeaponSystem::projectile_player_contact(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle collider)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// TODO: at the moment we are only checking damage, so if the player is already dead, we early out
		// Need other branches, e.g enemy which slows player
		auto player_component = component_manager.get_component<Player>(player);
		if (player_component->damage_timer > 0.0f)
		{
			// Player took damage previously, still timing out
			return;
		}

		auto player_health = component_manager.get_component<Health>(player);
		if (player_health.is_valid() == false)
		{
			return;
		}

		if (player_health->current_health <= 0.0f)
		{
			// Player is no longer alive
			return;
		}

		auto projectile = component_manager.get_component<ProjectileComponent>(collider);
		if (projectile.is_valid() == true)
		{
			// Apply damage
			player_health->current_health -= projectile->damage;

			// TODO: knockback?
		}

		// Restart damage timer
		// TODO: this will only allow the player to take damage from the first thing they collided with!
		// May need to revise to give certain damage sources priority!
		player_component->damage_timer = player_component->damage_delay;
	}
}