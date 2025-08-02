#include <VadonDemo/Model/Weapon/System.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

namespace
{
	struct CreateProjectileData
	{
		Vadon::ECS::EntityHandle weapon_entity;
		Vadon::Scene::SceneID prefab;
	};
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

		ProjectileComponent::register_component();
		ProjectileHomingComponent::register_component();
		ProjectileAOEComponent::register_component();
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
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		Vadon::Scene::SceneSystem& scene_system = engine_core.get_system<Vadon::Scene::SceneSystem>();

		// Have to defer creating the projectiles, because we can't add to the ECS while iterating in it
		// FIXME: implement deferred operations in ECS?
		std::vector<CreateProjectileData> deferred_projectiles;

		// TODO: revise below

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
				CreateProjectileData& new_projectile_data = deferred_projectiles.emplace_back();

				const WeaponDefHandle weapon_def_handle = resource_system.load_resource(current_weapon.definition);
				const WeaponDefinition* weapon_def = resource_system.get_resource(weapon_def_handle);

				new_projectile_data.weapon_entity = weapon_it.get_entity();
				new_projectile_data.prefab = weapon_def->projectile_prefab;

				// Reset firing timer
				current_weapon.firing_timer += 60.0f / std::max(weapon_def->rate_of_fire, 0.001f);
			}
		}

		if (deferred_projectiles.empty() == true)
		{
			return;
		}

		Vadon::ECS::EntityHandle root_entity;
		{
			auto level_root_query = component_manager.run_component_query<LevelRootTag&>();
			for (auto root_it = level_root_query.get_iterator(); (root_it.is_valid() == true) && (root_entity.is_valid() == false); root_it.next())
			{
				root_entity = root_it.get_entity();
			}
		}

		for (const CreateProjectileData& current_projectile_data : deferred_projectiles)
		{
			// Spawn projectile
			// FIXME: at the moment we make an entity for every projectile
			// Could use a special "pool" Entity that contains projectile instances
			const Vadon::Scene::SceneHandle spawned_projectile_scene = scene_system.load_scene(current_projectile_data.prefab);
			const Vadon::ECS::EntityHandle spawned_projectile = scene_system.instantiate_scene(spawned_projectile_scene, ecs_world);

			// FIXME: make a parent entity for projectiles?
			// Could be the projectile subsystem
			ecs_world.get_entity_manager().add_child_entity(root_entity, spawned_projectile);

			// Get the weapon that fired the projectile
			auto weapon_tuple = component_manager.get_component_tuple<Transform2D, WeaponComponent>(current_projectile_data.weapon_entity);

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
				else if(const Player* player_component = component_manager.get_component<Player>(current_projectile_data.weapon_entity))
				{
					// Choose the last direction the player moved in
					initial_direction = player_component->last_move_dir;
				}
			}
			else
			{
				if (const Player* player_component = component_manager.get_component<Player>(current_projectile_data.weapon_entity))
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
	}

	void WeaponSystem::update_projectiles(Vadon::ECS::World& ecs_world, float delta_time)
	{
		// Check if any projectiles timed out
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto projectile_query = component_manager.run_component_query<ProjectileComponent&>();
		for (auto projectile_it = projectile_query.get_iterator(); projectile_it.is_valid() == true; projectile_it.next())
		{
			auto projectile_tuple = projectile_it.get_tuple();
			ProjectileComponent& current_projectile = std::get<ProjectileComponent&>(projectile_tuple);
			current_projectile.remaining_lifetime -= delta_time;

			if (current_projectile.remaining_lifetime <= 0.0f)
			{
				// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
				// Whenever we spawn a projectile again, we can just "resurrect" a projectile that was already used
				component_manager.set_entity_tag<DestroyEntityTag>(projectile_it.get_entity(), true);
			}
		}
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
}