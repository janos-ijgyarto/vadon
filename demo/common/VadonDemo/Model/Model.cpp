#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Core/Core.hpp>
#include <VadonDemo/Model/Component.hpp>
#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/World/World.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

#include <numbers>

namespace
{
	Vadon::Math::Vector2 rotate_2d_vector(const Vadon::Math::Vector2& vector, float angle)
	{
		const float sin_angle = std::sinf(angle);
		const float cos_angle = std::cosf(angle);

		return Vadon::Math::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
	}

	bool collision_test(const Vadon::Math::Vector2& pos_a, float radius_a, const Vadon::Math::Vector2& pos_b, float radius_b)
	{
		const float hit_radius = radius_a + radius_b;
		const float hit_radius_sq = hit_radius * hit_radius;

		const Vadon::Math::Vector2 a_to_b = pos_b - pos_a;
		const float dist_sq = Vadon::Math::Vector::dot(a_to_b, a_to_b);

		return dist_sq < hit_radius_sq;
	}
}

namespace VadonDemo::Model
{
	void Model::register_types()
	{
		Vadon::ECS::ComponentRegistry::register_tag_type<LevelRootTag>();
		Vadon::ECS::ComponentRegistry::register_tag_type<DestroyEntityTag>();

		// FIXME: have to do this first because it gets referenced by player component
		// Need to create a system where we can set initialization order based on dependencies!
		WeaponSystem::register_types();

		Transform2D::register_component();
		Velocity2D::register_component();
		Collision::register_component();
		Health::register_component();
		Player::register_component();
		Enemy::register_component();
		Map::register_component();
		Spawner::register_component();
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
		update_enemies(ecs_world, delta_time);
		update_collisions(ecs_world, delta_time);
		update_health(ecs_world, delta_time);
		update_spawners(ecs_world, delta_time);

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
		ecs_world.get_entity_manager().remove_entity(root_entity);
		ecs_world.remove_pending_entities();
	}

	Vadon::ECS::EntityHandle Model::get_root_entity(Vadon::ECS::World& ecs_world)
	{
		Vadon::ECS::EntityHandle root_entity;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto level_root_query = component_manager.run_component_query<LevelRootTag&>();
		for (auto root_it = level_root_query.get_iterator(); (root_it.is_valid() == true) && (root_entity.is_valid() == false); root_it.next())
		{
			root_entity = root_it.get_entity();
		}

		return root_entity;
	}

	Model::Model(Core::Core& core)
		: m_core(core)
		, m_random_engine(std::random_device{}())
		, m_enemy_dist(0.0f, 2 * std::numbers::pi_v<float>)
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
				auto player_tuple = player_it.get_tuple();

				const Player& player_component = std::get<Player&>(player_tuple);
				WeaponComponent& player_weapon = std::get<WeaponComponent&>(player_tuple);

				std::uniform_int_distribution<int> m_int_distribution(0, static_cast<int>(player_component.starting_weapons.size()) - 1);

				player_weapon.definition = player_component.starting_weapons[m_int_distribution(m_random_engine)];
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
				auto player_components = component_manager.get_component_tuple<Transform2D, Velocity2D, Health, WeaponComponent>(player_entity);
				if (std::get<Transform2D*>(player_components) == nullptr)
				{
					// TODO: error!
					return false;
				}
				if (std::get<Velocity2D*>(player_components) == nullptr)
				{
					// TODO: error!
					return false;
				}

				Health* player_health = std::get<Health*>(player_components);
				if (player_health == nullptr)
				{
					// TODO: error!
					return false;
				}

				player_health->current_health = player_health->max_health;

				WeaponComponent* player_weapon = std::get<WeaponComponent*>(player_components);
				if (player_weapon == nullptr)
				{
					// TODO: error!
					return false;
				}

				auto player_tuple = player_it.get_tuple();
				if (m_weapon_system.validate_weapon(std::get<Player&>(player_tuple), *player_weapon) == false)
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

				auto map_components = component_manager.get_component_tuple<Transform2D, Map>(map_entity);
				if (std::get<Map*>(map_components) == nullptr)
				{
					// TODO: error!
					return false;
				}
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
				auto spawner_tuple = spawner_it.get_tuple();
				Spawner& current_spawner = std::get<Spawner&>(spawner_tuple);

				if (validate_spawner(current_spawner) == false)
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

	bool Model::validate_spawner(const Spawner& spawner)
	{
		if (spawner.enemy_prefab.is_valid() == false)
		{
			// TODO: error!
			return false;
		}

		return true;
	}

	void Model::update_player(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto player_query = component_manager.run_component_query<Player&, Velocity2D*>();
		for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
		{
			auto player_tuple = player_it.get_tuple();
			Player& player_component = std::get<Player&>(player_tuple);

			if (player_component.damage_timer > 0.0f)
			{
				player_component.damage_timer = std::max(0.0f, player_component.damage_timer - delta_time);
			}

			Velocity2D* player_velocity = std::get<Velocity2D*>(player_tuple);
			Vadon::Math::Vector2 facing = player_component.last_move_dir;
			if (player_velocity != nullptr)
			{
				// TODO: use acceleration!
				player_velocity->velocity = player_component.input.move_dir * player_velocity->top_speed;
				if (player_component.input.move_dir != Vadon::Math::Vector2_Zero)
				{
					facing = Vadon::Math::Vector::normalize(player_component.input.move_dir);
				}
			}
			player_component.last_move_dir = facing;
		}
	}

	void Model::update_enemies(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		// FIXME: instead of having to do a query, we should just cache the player entity handle!
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
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
		const float teleport_radius_sq = Vadon::Math::Vector::length_squared(global_config.viewport_size * 0.55f);

		auto enemy_query = component_manager.run_component_query<Enemy&, Transform2D&, Velocity2D&>();
		for (auto enemy_it = enemy_query.get_iterator(); enemy_it.is_valid() == true; enemy_it.next())
		{
			auto enemy_tuple = enemy_it.get_tuple();
			Transform2D& enemy_transform = std::get<Transform2D&>(enemy_tuple);
			Velocity2D& enemy_velocity = std::get<Velocity2D&>(enemy_tuple);

			const Vadon::Math::Vector2 enemy_to_player = player_position - enemy_transform.position;
			if (Vadon::Math::Vector::dot(enemy_to_player, enemy_to_player) > 0.01f)
			{
				enemy_velocity.velocity = Vadon::Math::Vector::normalize(enemy_to_player) * enemy_velocity.top_speed;
			}
			else
			{
				enemy_velocity.velocity = Vadon::Math::Vector2_Zero;
			}

			const float enemy_dist_sq = Vadon::Math::Vector::length_squared(enemy_to_player);
			if (enemy_dist_sq > teleport_radius_sq)
			{
				const Vadon::Math::Vector2 new_position = player_position + (enemy_to_player * 0.9f);
				enemy_transform.position = new_position;
				enemy_transform.teleported = true;
			}
		}
	}

	void Model::update_dynamic(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto dynamic_entity_query = component_manager.run_component_query<Transform2D&, Velocity2D&>();
		for (auto dynamic_entity_it = dynamic_entity_query.get_iterator(); dynamic_entity_it.is_valid() == true; dynamic_entity_it.next())
		{
			auto dynamic_tuple = dynamic_entity_it.get_tuple();
			Transform2D& current_transform = std::get<Transform2D&>(dynamic_tuple);
			const Velocity2D& current_velocity = std::get<Velocity2D&>(dynamic_tuple);

			current_transform.position += current_velocity.velocity * delta_time;
			current_transform.teleported = false;
		}
	}

	void Model::update_collisions(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		// FIXME: this is just a placeholder for an actual collision system
		// Could just use ECS, but the long-term plan is to use accelerator structures for processes like this
		enum class CollisionType
		{
			PLAYER,
			ENEMY,
			PROJECTILE,
			OTHER
		};

		struct CollisionData
		{
			Vadon::ECS::EntityHandle entity;
			Vadon::Math::Vector2 position;
			float radius = 0.0f;
			CollisionType type = CollisionType::OTHER;
			// TODO: some other metadata?
		};
		std::vector<CollisionData> collision_data_vec;

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto collision_query = component_manager.run_component_query<Transform2D&, Collision&>();
		for (auto collision_it = collision_query.get_iterator(); collision_it.is_valid() == true; collision_it.next())
		{
			auto collision_tuple = collision_it.get_tuple();
			const Transform2D& current_transform = std::get<Transform2D&>(collision_tuple);
			Collision& current_collision = std::get<Collision&>(collision_tuple);

			current_collision.collision_data_offset = collision_data_vec.size();

			CollisionData& collision_data = collision_data_vec.emplace_back();
			collision_data.entity = collision_it.get_entity();
			collision_data.position = current_transform.position;
			collision_data.radius = current_transform.scale * current_collision.radius;

			if (component_manager.has_component<Player>(collision_it.get_entity()) == true)
			{
				collision_data.type = CollisionType::PLAYER;
			}
			else if (component_manager.has_component<Enemy>(collision_it.get_entity()) == true)
			{
				collision_data.type = CollisionType::ENEMY;
			}
			else if (component_manager.has_component<ProjectileComponent>(collision_it.get_entity()) == true)
			{
				collision_data.type = CollisionType::PROJECTILE;
			}
		}

		for (const CollisionData& current_collision : collision_data_vec)
		{
			switch (current_collision.type)
			{
			case CollisionType::PLAYER:
			{
				Player* player_component = component_manager.get_component<Player>(current_collision.entity);
				if (player_component->damage_timer > 0.0f)
				{
					// Player took damage previously, still timing out
					continue;
				}

				Health* player_health = component_manager.get_component<Health>(current_collision.entity);
				if (player_health == nullptr)
				{
					continue;
				}

				if (player_health->current_health <= 0.0f)
				{
					// Player is no longer alive
					continue;
				}

				// Check against all enemies
				for (const CollisionData& other_collision : collision_data_vec)
				{
					if (other_collision.type != CollisionType::ENEMY)
					{
						continue;
					}

					// End on the first collision
					// FIXME: should we check all and take the closest hit instead?
					if (collision_test(current_collision.position, current_collision.radius, other_collision.position, other_collision.radius))
					{
						const Enemy* enemy_component = component_manager.get_component<Enemy>(other_collision.entity);

						// Apply damage
						player_health->current_health -= enemy_component->damage;

						// Restart damage timer
						player_component->damage_timer = player_component->damage_delay;
						break;
					}
				}
			}
			break;
			case CollisionType::ENEMY:
			{
				Health* enemy_health = component_manager.get_component<Health>(current_collision.entity);
				if (enemy_health == nullptr)
				{
					continue;
				}

				if (enemy_health->current_health <= 0.0f)
				{
					// Enemy is no longer alive
					continue;
				}

				// Check against all projectiles
				for (const CollisionData& other_collision : collision_data_vec)
				{
					if (other_collision.type != CollisionType::PROJECTILE)
					{
						continue;
					}

					ProjectileComponent* projectile = component_manager.get_component<ProjectileComponent>(other_collision.entity);
					if (projectile->remaining_lifetime <= 0.0f)
					{
						// Projectile has already expired
						continue;
					}

					if (collision_test(current_collision.position, current_collision.radius, other_collision.position, other_collision.radius))
					{
						// Make sure projectile "dies" from this impact
						projectile->remaining_lifetime = 0.0f;

						// Apply damage
						enemy_health->current_health -= projectile->damage;
					}

					if (enemy_health->current_health <= 0.0f)
					{
						// Enemy is no longer alive
						break;
					}
				}
			}
			break;
			}
		}
	}

	void Model::update_health(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto health_query = component_manager.run_component_query<Health&>();
		for (auto health_it = health_query.get_iterator(); health_it.is_valid() == true; health_it.next())
		{
			auto health_tuple = health_it.get_tuple();
			const Health& health_component = std::get<Health&>(health_tuple);

			if (health_component.current_health <= 0.0f)
			{
				// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
				// Whenever we spawn an entity again, we can just "resurrect" one that was already spawned
				component_manager.set_entity_tag<DestroyEntityTag>(health_it.get_entity(), true);
			}
		}
	}

	void Model::update_spawners(Vadon::ECS::World& ecs_world, float delta_time)
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
						const float rand_angle = m_enemy_dist(m_random_engine);

						// Spawn relative to player
						const Vadon::Math::Vector2 new_enemy_pos = player_position + Vadon::Math::Vector2(std::cosf(rand_angle), std::sinf(rand_angle)) * spawn_radius;
						spawn_enemy(ecs_world, current_spawner, new_enemy_pos);
					}
				}
			}
		}
	}

	void Model::spawn_enemy(Vadon::ECS::World& ecs_world, const Spawner& spawner, const Vadon::Math::Vector2& position)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Scene::SceneSystem& scene_system = m_core.get_engine_core().get_system<Vadon::Scene::SceneSystem>();

		const Vadon::Scene::SceneHandle enemy_prefab_scene = scene_system.load_scene(spawner.enemy_prefab);
		const Vadon::ECS::EntityHandle spawned_enemy = scene_system.instantiate_scene(enemy_prefab_scene, ecs_world);

		// FIXME: make a parent entity for enemies?
		// Could be the enemy subsystem
		Vadon::ECS::EntityHandle root_entity = get_root_entity(ecs_world);
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

		m_core.entity_added(ecs_world, spawned_enemy);
	}

	void Model::clear_removed_entities(Vadon::ECS::World& ecs_world)
	{
		// Find all tagged entities, mark them for removal
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto destroy_entity_query = component_manager.run_component_query<DestroyEntityTag&>();
		for (auto entity_it = destroy_entity_query.get_iterator(); entity_it.is_valid() == true; entity_it.next())
		{
			m_core.entity_removed(ecs_world, entity_it.get_entity());
			entity_manager.remove_entity(entity_it.get_entity());
		}

		// Purge from the ECS world completely
		ecs_world.remove_pending_entities();
	}
}