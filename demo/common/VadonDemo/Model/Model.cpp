#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Enemy/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

namespace VadonDemo::Model
{
	void Model::register_types()
	{
		Vadon::ECS::ComponentRegistry::register_tag_type<LevelRootTag>();
		Vadon::ECS::ComponentRegistry::register_tag_type<DestroyEntityTag>();

		// FIXME: have to do this first because it gets referenced by player component
		// Need to create a system where we can set initialization order based on dependencies!
		WeaponSystem::register_types();
		EnemySystem::register_types();
		CollisionSystem::register_types();

		Transform2D::register_component();
		Velocity2D::register_component();
		Health::register_component();
		Player::register_component();
		Map::register_component();
	}

	bool Model::init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id)
	{
		if (internal_init_simulation(ecs_world, level_scene_id) == false)
		{
			end_simulation(ecs_world);
			return false;
		}

		return true;
	}

	void Model::update(Vadon::ECS::World& ecs_world, float delta_time)
	{
		// TODO: implement some kind of dependency logic?
		// In the long-term, need to introduce the System part of ECS, where component processing is scheduled based on dependencies
		update_dynamic(ecs_world, delta_time);
		update_player(ecs_world, delta_time);
		
		m_collision_system.update(ecs_world, delta_time);
		
		update_health(ecs_world, delta_time);

		m_enemy_system.update(ecs_world, delta_time);

		m_weapon_system.update(ecs_world, delta_time);

		clear_removed_entities(ecs_world);

		m_elapsed_time += delta_time;
	}

	bool Model::is_in_end_state(Vadon::ECS::World& ecs_world) const
	{
		Vadon::ECS::EntityHandle player_entity;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto player_query = component_manager.run_component_query<Player&>();
		for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
		{
			player_entity = player_it.get_entity();
			if (player_entity.is_valid() == true)
			{
				break;
			}
		}

		if (player_entity.is_valid() == false)
		{
			return true;
		}

		// TODO2: check some kind of victory condition?
		return false;
	}

	void Model::end_simulation(Vadon::ECS::World& ecs_world)
	{
		Vadon::ECS::EntityHandle root_entity = get_root_entity(ecs_world);
		if (root_entity.is_valid() == false)
		{
			return;
		}

		m_core.entity_removed(ecs_world, root_entity);

		// TODO: error checking?
		ecs_world.remove_entity(root_entity);
	}

	Vadon::ECS::EntityHandle Model::get_root_entity(Vadon::ECS::World& ecs_world)
	{
		// FIXME: could just store this explicitly, the model is expected to keep track of this anyway
		Vadon::ECS::EntityHandle root_entity;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto level_root_query = component_manager.run_component_query<LevelRootTag&>();
		for (auto root_it = level_root_query.get_iterator(); (root_it.is_valid() == true) && (root_entity.is_valid() == false); root_it.next())
		{
			root_entity = root_it.get_entity();
		}

		return root_entity;
	}

	void Model::init_entity_collision(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto collision_component = component_manager.get_component<Collision>(entity);
		if (collision_component.is_valid() == false)
		{
			return;
		}

		if(component_manager.has_component<Player>(entity))
		{
			collision_component->callback = &Model::player_collision_callback;
			collision_component->layers = 1 << static_cast<uint8_t>(CollisionLayer::PLAYER);
			return;
		}

		if (m_enemy_system.init_collisions(ecs_world, entity) == true)
		{
			return;
		}

		auto projectile_component = component_manager.get_component<ProjectileComponent>(entity);
		if(projectile_component.is_valid() == true)
		{
			collision_component->callback = &WeaponSystem::projectile_collision_callback;
			collision_component->layers = 1 << static_cast<uint8_t>(CollisionLayer::PROJECTILES);
			if (projectile_component->enemy == false)
			{
				collision_component->mask = 1 << static_cast<uint8_t>(CollisionLayer::ENEMIES);
			}
			else
			{
				collision_component->mask = 1 << static_cast<uint8_t>(CollisionLayer::PLAYER);
			}
			return;
		}
	}

	Model::Model(Core::Core& core)
		: m_core(core)
		, m_random_engine(std::random_device{}())
		, m_collision_system(core)
		, m_enemy_system(core)
		, m_weapon_system(core)
		, m_elapsed_time(0.0f)
	{
	}

	bool Model::initialize()
	{
		// TODO: anything?
		return true;
	}

	void Model::global_config_updated()
	{
		// TODO: anything?
	}

	bool Model::internal_init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id)
	{
		if (load_level(ecs_world, level_scene_id) == false)
		{
			// TODO: error!
			return false;
		}

		if (validate_sim_state(ecs_world) == false)
		{
			return false;
		}

		// Randomly select one of the player's starting weapons
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			Vadon::ECS::EntityHandle player_entity;
			auto player_query = component_manager.run_component_query<Player&, WeaponComponent&>();
			for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
			{
				player_entity = player_it.get_entity();
				const auto player_component = player_it.get_component<Player>();
				auto player_weapon = player_it.get_component<WeaponComponent>();

				std::uniform_int_distribution<int> m_int_distribution(0, static_cast<int>(player_component->starting_weapons.size()) - 1);

				player_weapon->definition = player_component->starting_weapons[m_int_distribution(m_random_engine)];
			}
		}

		return true;
	}

	bool Model::load_level(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id)
	{
		Vadon::ECS::EntityHandle root_entity = get_root_entity(ecs_world);
		VADON_ASSERT(root_entity.is_valid() == false, "Cannot load a level while a game is already in progress!");

		Vadon::Scene::SceneSystem& scene_system = m_core.get_engine_core().get_system<Vadon::Scene::SceneSystem>();
		const Vadon::Scene::SceneHandle level_scene_handle = scene_system.load_scene(level_scene_id);
		if (level_scene_handle.is_valid() == false)
		{
			// Something went wrong
			return false;
		}

		root_entity = scene_system.instantiate_scene(level_scene_handle, ecs_world);
		if (root_entity.is_valid() == false)
		{
			// Something went wrong
			return false;
		}

		// Tag the root
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		component_manager.set_entity_tag<LevelRootTag>(root_entity, true);

		m_core.entity_added(ecs_world, root_entity);
		return true;
	}

	bool Model::validate_sim_state(Vadon::ECS::World& ecs_world)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		// Validate player and map
		// TODO: alterantively, could make it a required setting in the scene?
		// Some kind of special scene which spawns a player and map?
		{
			Vadon::ECS::EntityHandle player_entity;
			auto player_query = component_manager.run_component_query<Player&>();
			for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
			{
				if (player_entity.is_valid() == true)
				{
					// TODO: error!
					return false;
				}

				player_entity = player_it.get_entity();
				const auto player_transform = component_manager.get_component<Transform2D>(player_entity);
				if (player_transform.is_valid() == false)
				{
					// TODO: error!
					return false;
				}
				const auto player_velocity = component_manager.get_component<Velocity2D>(player_entity);
				if (player_velocity.is_valid() == false)
				{
					// TODO: error!
					return false;
				}

				auto player_health = component_manager.get_component<Health>(player_entity);
				if (player_health.is_valid() == false)
				{
					// TODO: error!
					return false;
				}

				player_health->current_health = player_health->max_health;

				auto player_weapon = component_manager.get_component<WeaponComponent>(player_entity);
				if (player_weapon.is_valid() == false)
				{
					// TODO: error!
					return false;
				}
				
				auto player_component = player_it.get_component<Player>();
				if (m_weapon_system.validate_weapon(player_component, *player_weapon) == false)
				{
					// TODO: error!
					return false;
				}
			}

			if (player_entity.is_valid() == false)
			{
				// TODO: error!
				return false;
			}
		}

		{
			Vadon::ECS::EntityHandle map_entity;
			auto map_query = component_manager.run_component_query<Map&>();

			for (auto map_it = map_query.get_iterator(); map_it.is_valid() == true; map_it.next())
			{
				if (map_entity.is_valid() == true)
				{
					// TODO: error!
					return false;
				}

				map_entity = map_it.get_entity();
			}

			if (map_entity.is_valid() == false)
			{
				// TODO: error!
				return false;
			}
		}

		{
			auto spawner_query = component_manager.run_component_query<Spawner&>();
			bool spawner_loaded = false;

			for (auto spawner_it = spawner_query.get_iterator(); spawner_it.is_valid() == true; spawner_it.next())
			{
				auto current_spawner = spawner_it.get_component<Spawner>();

				if (m_enemy_system.validate_spawner(current_spawner) == false)
				{
					// TODO: error message about incorrectly configured spawner!
					return false;
				}

				spawner_loaded = true;
			}
			if (spawner_loaded == false)
			{
				// TODO: error!
				return false;
			}
		}

		return true;
	}

	void Model::update_player(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto player_query = component_manager.run_component_query<Player&, Velocity2D*, WeaponComponent*>();
		for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
		{
			auto player_component = player_it.get_component<Player>();

			if (player_component->damage_timer > 0.0f)
			{
				player_component->damage_timer = std::max(0.0f, player_component->damage_timer - delta_time);
			}

			auto player_velocity = player_it.get_component<Velocity2D>();
			Vadon::Math::Vector2 facing = player_component->last_move_dir;
			if (player_velocity.is_valid() == true)
			{
				// TODO: use acceleration!
				player_velocity->velocity = player_component->input.move_dir * player_velocity->top_speed;
				if (player_component->input.move_dir != Vadon::Math::Vector2_Zero)
				{
					facing = Vadon::Math::Vector::normalize(player_component->input.move_dir);
				}
			}
			player_component->last_move_dir = facing;
		}
	}

	void Model::update_dynamic(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto dynamic_entity_query = component_manager.run_component_query<Transform2D&, Velocity2D&>();
		for (auto dynamic_entity_it = dynamic_entity_query.get_iterator(); dynamic_entity_it.is_valid() == true; dynamic_entity_it.next())
		{
			auto current_transform = dynamic_entity_it.get_component<Transform2D>();
			const auto current_velocity = dynamic_entity_it.get_component<Velocity2D>();

			current_transform->position += current_velocity->velocity * delta_time;
			current_transform->teleported = false;
		}
	}

	void Model::update_health(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto health_query = component_manager.run_component_query<Health&>();
		for (auto health_it = health_query.get_iterator(); health_it.is_valid() == true; health_it.next())
		{
			const auto health_component = health_it.get_component<Health>();

			if (health_component->current_health <= 0.0f)
			{
				// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
				// Whenever we spawn an entity again, we can just "resurrect" one that was already spawned
				component_manager.set_entity_tag<DestroyEntityTag>(health_it.get_entity(), true);
			}
		}
	}

	void Model::player_collision_callback(VadonDemo::Core::Core& core, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle collider)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto player_component = component_manager.get_component<Player>(player);
		VADON_ASSERT(player_component.is_valid() == true, "Cannot find player component!");

		// Check if we collided with enemy
		const auto enemy_component = component_manager.get_component<EnemyBase>(collider);
		if (enemy_component.is_valid() == true)
		{
			core.get_model().m_enemy_system.enemy_player_contact(ecs_world, player, collider);
		}

		// Check if we collided with projectile
		const auto projectile_component = component_manager.get_component<ProjectileComponent>(collider);
		if (projectile_component.is_valid() == true)
		{
			core.get_model().m_weapon_system.projectile_player_contact(ecs_world, player, collider);
		}
	}

	void Model::clear_removed_entities(Vadon::ECS::World& ecs_world)
	{
		// Find all tagged entities, mark them for removal
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto destroy_entity_query = component_manager.run_component_query<DestroyEntityTag&>();
		for (auto entity_it = destroy_entity_query.get_iterator(); entity_it.is_valid() == true; entity_it.next())
		{
			m_core.entity_removed(ecs_world, entity_it.get_entity());
			ecs_world.remove_entity(entity_it.get_entity());
		}
	}
}