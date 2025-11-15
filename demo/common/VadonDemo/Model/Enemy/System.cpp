#include <VadonDemo/Model/Enemy/System.hpp>

#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Collision/Component.hpp>
#include <VadonDemo/Model/Enemy/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <numbers>

namespace VadonDemo::Model
{
	EnemySystem::EnemySystem(Core::Core& core)
		: m_core(core)
		, m_enemy_dist(0.0f, 2 * std::numbers::pi_v<float>)
	{

	}

	void EnemySystem::register_types()
	{
		EnemyDefinition::register_resource();
		EnemyMovementDefinition::register_resource();
		EnemyMovementLookahead::register_resource();
		EnemyMovementWeaving::register_resource();
		EnemyWeaponAttackDefinition::register_resource();
		EnemyContactDamageDefinition::register_resource();

		EnemyBase::register_component();
		EnemyMovement::register_component();
		EnemyWeapon::register_component();
		EnemyContactDamage::register_component();
		
		Spawner::register_component();
	}

	bool EnemySystem::init_collisions(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Collision* collision_component = component_manager.get_component<Collision>(entity);
		if (collision_component == nullptr)
		{
			return false;
		}

		if (component_manager.has_component<EnemyBase>(entity))
		{
			collision_component->callback = &EnemySystem::enemy_collision_callback;
			collision_component->layers = 1 << static_cast<uint8_t>(CollisionLayer::ENEMIES);
			collision_component->mask = 1 << static_cast<uint8_t>(CollisionLayer::PLAYER);
			return true;
		}

		return false;
	}

	void EnemySystem::enemy_collision_callback(VadonDemo::Core::Core& /*core*/, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle collider)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Health* enemy_health = component_manager.get_component<Health>(enemy);
		if (enemy_health == nullptr)
		{
			return;
		}

		if (enemy_health->current_health <= 0.0f)
		{
			// Enemy is no longer alive
			return;
		}

		ProjectileComponent* projectile = component_manager.get_component<ProjectileComponent>(collider);
		if (projectile != nullptr)
		{
			// Apply damage
			enemy_health->current_health -= projectile->damage;

			if (projectile->knockback > 0.001f)
			{
				const Transform2D* projectile_transform = component_manager.get_component<Transform2D>(collider);
				Transform2D* enemy_transform = component_manager.get_component<Transform2D>(enemy);

				const Vadon::Math::Vector2 projectile_to_enemy = Vadon::Math::Vector::normalize(enemy_transform->position - projectile_transform->position);
				enemy_transform->position += (projectile_to_enemy * projectile->knockback);
			}
		}
	}

	bool EnemySystem::validate_sim_state() const
	{
		return false;
	}

	bool EnemySystem::validate_spawner(const Spawner& spawner) const
	{
		if (spawner.enemy_prefab.is_valid() == false)
		{
			// TODO: error!
			return false;
		}

		return true;
	}

	void EnemySystem::update(Vadon::ECS::World& ecs_world, float delta_time)
	{
		update_enemies(ecs_world, delta_time);
		update_spawners(ecs_world, delta_time);
	}

	void EnemySystem::update_enemies(Vadon::ECS::World& ecs_world, float delta_time)
	{
		// FIXME: instead of having to do a query, we should just cache the player entity handle!
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		Vadon::ECS::EntityHandle player_entity;
		Vadon::Math::Vector2 player_position = Vadon::Math::Vector2_Zero;
		{
			auto player_query = component_manager.run_component_query<Player&, Transform2D&>();
			for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true;)
			{
				player_entity = player_it.get_entity();

				auto player_tuple = player_it.get_tuple();
				Transform2D& player_transform = std::get<Transform2D&>(player_tuple);

				player_position = player_transform.position;
				break;
			}
		}

		const Core::GlobalConfiguration& global_config = m_core.get_global_config();
		const float teleport_radius_sq = Vadon::Math::Vector::length_squared(global_config.viewport_size * 0.55f);

		auto enemy_query = component_manager.run_component_query<EnemyBase&, Transform2D*, Velocity2D*, EnemyMovement*>();
		for (auto enemy_it = enemy_query.get_iterator(); enemy_it.is_valid() == true; enemy_it.next())
		{
			auto enemy_tuple = enemy_it.get_tuple();

			Velocity2D* velocity_component = std::get<Velocity2D*>(enemy_tuple);
			const EnemyMovement* movement_component = std::get<EnemyMovement*>(enemy_tuple);
			if ((player_entity.is_valid() == true) && (velocity_component != nullptr) && (movement_component != nullptr))
			{
				Vadon::Scene::ResourceSystem& resource_system = m_core.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
				const EnemyMovementDefinition* movement_def = resource_system.get_resource(movement_component->def_handle);

				velocity_component->velocity = movement_def->get_movement_direction(ecs_world, enemy_it.get_entity(), player_entity, delta_time) * velocity_component->top_speed;
			}
			else if (velocity_component != nullptr)
			{
				velocity_component->velocity = Vadon::Math::Vector2_Zero;
			}

			Transform2D* transform_component = std::get<Transform2D*>(enemy_tuple);
			if (transform_component != nullptr)
			{
				const Vadon::Math::Vector2 enemy_to_player = player_position - transform_component->position;
				const float enemy_dist_sq = Vadon::Math::Vector::length_squared(enemy_to_player);
				if (enemy_dist_sq > teleport_radius_sq)
				{
					const Vadon::Math::Vector2 new_position = player_position + (enemy_to_player * 0.9f);
					transform_component->position = new_position;
					transform_component->teleported = true;
				}
			}
		}
	}

	void EnemySystem::update_spawners(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// FIXME: instead of having to do a query, we should just cache the player entity handle!
		Vadon::Math::Vector2 player_position = Vadon::Math::Vector2_Zero;

		{
			auto player_query = component_manager.run_component_query<Player&, Transform2D&>();
			for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true;)
			{
				auto player_tuple = player_it.get_tuple();
				Transform2D& player_transform = std::get<Transform2D&>(player_tuple);

				player_position = player_transform.position;
				break;
			}
		}

		const Core::GlobalConfiguration& global_config = m_core.get_global_config();
		const float spawn_radius = Vadon::Math::Vector::length(global_config.viewport_size * 0.5f);

		// NOTE: currently all spawners just keep spawning once the delay counts down
		// Need to implement limits on enemy numbers and a priority system
		// Can put all spawners that timed out into a FIFO list
		auto spawner_query = component_manager.run_component_query<Spawner&>();
		for (auto spawner_it = spawner_query.get_iterator(); spawner_it.is_valid() == true; spawner_it.next())
		{
			auto spawner_tuple = spawner_it.get_tuple();
			Spawner& current_spawner = std::get<Spawner&>(spawner_tuple);

			// Check if spawner is ready to activate
			// TODO: move delay to def, use component with timer, remove component once activated
			if (current_spawner.activation_delay > 0.0f)
			{
				current_spawner.activation_delay -= delta_time;
			}
			else
			{
				// Update level up timer
				if (current_spawner.current_level < current_spawner.max_level)
				{
					current_spawner.level_up_timer -= delta_time;
					if (current_spawner.level_up_timer <= 0.0f)
					{
						++current_spawner.current_level;
						current_spawner.level_up_timer = std::max(current_spawner.level_up_timer + current_spawner.level_up_delay, 0.0f);
					}
				}

				current_spawner.spawn_timer -= delta_time;
				if (current_spawner.spawn_timer <= 0.0f)
				{
					current_spawner.spawn_timer = std::max(current_spawner.spawn_timer + current_spawner.min_spawn_delay, 0.0f);

					for (int32_t spawned_enemy_index = 0; spawned_enemy_index < current_spawner.current_spawn_count; ++spawned_enemy_index)
					{
						const float rand_angle = m_enemy_dist(m_core.get_model().get_random_engine());

						// Spawn relative to player
						const Vadon::Math::Vector2 new_enemy_pos = player_position + Vadon::Math::Vector2(std::cosf(rand_angle), std::sinf(rand_angle)) * spawn_radius;
						spawn_enemy(ecs_world, current_spawner, new_enemy_pos);
					}
				}
			}
		}
	}

	void EnemySystem::spawn_enemy(Vadon::ECS::World& ecs_world, const Spawner& spawner, const Vadon::Math::Vector2& position)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Scene::SceneSystem& scene_system = m_core.get_engine_core().get_system<Vadon::Scene::SceneSystem>();

		const Vadon::Scene::SceneHandle enemy_prefab_scene = scene_system.load_scene(spawner.enemy_prefab);
		const Vadon::ECS::EntityHandle spawned_enemy = scene_system.instantiate_scene(enemy_prefab_scene, ecs_world);

		// FIXME: make a parent entity for enemies?
		// Could be the enemy subsystem
		Vadon::ECS::EntityHandle root_entity = Model::get_root_entity(ecs_world);
		ecs_world.get_entity_manager().add_child_entity(root_entity, spawned_enemy);

		Transform2D* transform_component = component_manager.get_component<Transform2D>(spawned_enemy);

		if (transform_component != nullptr)
		{
			// Spawn relative to player
			transform_component->position = position;
		}

		// Init health
		// FIXME: could use event for this!
		Health* health_component = component_manager.get_component<Health>(spawned_enemy);
		if (health_component != nullptr)
		{
			health_component->current_health = health_component->max_health;
		}

		EnemyMovement* movement_component = component_manager.get_component<EnemyMovement>(spawned_enemy);
		if (movement_component != nullptr)
		{
			if (movement_component->definition.is_valid() == true)
			{
				Vadon::Scene::ResourceSystem& resource_system = m_core.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
				movement_component->def_handle = resource_system.load_resource(movement_component->definition);
			}
		}

		m_core.entity_added(ecs_world, spawned_enemy);
	}

	void EnemySystem::enemy_player_contact(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle enemy)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// TODO: at the moment we are only checking damage, so if the player is already dead, we early out
		// Need other branches, e.g enemy which slows player
		Player* player_component = component_manager.get_component<Player>(player);
		if (player_component->damage_timer > 0.0f)
		{
			// Player took damage previously, still timing out
			return;
		}

		Health* player_health = component_manager.get_component<Health>(player);
		if (player_health == nullptr)
		{
			return;
		}

		if (player_health->current_health <= 0.0f)
		{
			// Player is no longer alive
			return;
		}

		EnemyContactDamage* contact_damage = component_manager.get_component<EnemyContactDamage>(enemy);
		if (contact_damage == nullptr)
		{
			return;
		}

		if (contact_damage->definition.is_valid() == false)
		{
			return;
		}

		// FIXME: Cache handle instead of having to check each time?
		Vadon::Scene::ResourceSystem& resource_system = m_core.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		EnemyContactDamageDefHandle contact_damage_def_handle = resource_system.load_resource(contact_damage->definition);

		const EnemyContactDamageDefinition* contact_damage_def = resource_system.get_resource(contact_damage_def_handle);

		// Apply damage
		player_health->current_health -= contact_damage_def->damage;

		// Restart damage timer
		// TODO: this will only allow the player to take damage from the first thing they collided with!
		// May need to revise to give certain damage sources priority!
		player_component->damage_timer = player_component->damage_delay;
	}
}