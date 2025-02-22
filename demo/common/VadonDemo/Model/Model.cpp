#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <Vadon/Core/CoreInterface.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/SceneSystem.hpp>

#include <numbers>

namespace
{
	Vadon::Utilities::Vector2 rotate_2d_vector(const Vadon::Utilities::Vector2& vector, float angle)
	{
		const float sin_angle = std::sinf(angle);
		const float cos_angle = std::cosf(angle);

		return Vadon::Utilities::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
	}

	bool collision_test(const Vadon::Utilities::Vector2& pos_a, float radius_a, const Vadon::Utilities::Vector2& pos_b, float radius_b)
	{
		const float hit_radius = radius_a + radius_b;
		const float hit_radius_sq = hit_radius * hit_radius;

		const Vadon::Utilities::Vector2 a_to_b = pos_b - pos_a;
		const float dist_sq = Vadon::Utilities::dot(a_to_b, a_to_b);

		return dist_sq < hit_radius_sq;
	}
}

namespace VadonDemo::Model
{
	void Model::register_types()
	{
		Transform2D::register_component();
		Velocity2D::register_component();
		Collision::register_component();
		Health::register_component();
		Player::register_component();
		Weapon::register_component();
		Projectile::register_component();
		Enemy::register_component();
		Map::register_component();
		Spawner::register_component();
	}

	bool Model::init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id)
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

		return true;
	}

	void Model::update(Vadon::ECS::World& ecs_world, float delta_time)
	{
		update_player(ecs_world, delta_time);
		update_enemies(ecs_world, delta_time);
		update_dynamic(ecs_world, delta_time);
		update_collisions(ecs_world, delta_time);
		update_health(ecs_world, delta_time);
		update_spawners(ecs_world, delta_time);
		update_weapons(ecs_world, delta_time);
		update_projectiles(ecs_world, delta_time);

		clear_removed_entities(ecs_world);
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
		if (m_level_root.is_valid() == false)
		{
			return;
		}

		// TODO: error checking?
		ecs_world.remove_entity(m_level_root);
		m_level_root.invalidate();
	}

	Model::Model(Vadon::Core::EngineCoreInterface& engine_core)
		: m_engine_core(engine_core)
		, m_random_engine(std::random_device{}())
		, m_enemy_dist(0.0f, 2 * std::numbers::pi_v<float>)
	{
	}

	bool Model::initialize()
	{
		// TODO: anything?
		return true;
	}

	bool Model::load_level(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id)
	{
		VADON_ASSERT(m_level_root.is_valid() == false, "Cannot load a level while a game is already in progress!");

		Vadon::Scene::SceneSystem& scene_system = m_engine_core.get_system<Vadon::Scene::SceneSystem>();
		const Vadon::Scene::SceneHandle level_scene_handle = scene_system.load_scene(level_scene_id);
		if (level_scene_handle.is_valid() == false)
		{
			// Something went wrong
			return false;
		}

		m_level_root = scene_system.instantiate_scene(level_scene_handle, ecs_world);
		if (m_level_root.is_valid() == false)
		{
			// Something went wrong
			return false;
		}

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
				auto player_components = component_manager.get_component_tuple<Transform2D, Velocity2D, Health, Weapon>(player_entity);
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

				Weapon* player_weapon = std::get<Weapon*>(player_components);
				if (player_weapon == nullptr)
				{
					// TODO: error!
					return false;
				}

				if (validate_weapon(*player_weapon) == false)
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
				if (std::get<Transform2D*>(map_components) == nullptr)
				{
					// TODO: error!
					return false;
				}
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

	bool Model::validate_weapon(const Weapon& weapon_component)
	{
		if (weapon_component.projectile_prefab.is_valid() == false)
		{
			// TODO: error!
			return false;
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

	void Model::deferred_remove_entity(Vadon::ECS::EntityHandle entity_handle)
	{
		// FIXME: should we also mark the entity as "removed" so it doesn't get used?
		// TODO: sort and use lower_bound?
		if (std::find(m_entity_remove_list.begin(), m_entity_remove_list.end(), entity_handle) != m_entity_remove_list.end())
		{
			// Already added
			return;
		}

		m_entity_remove_list.push_back(entity_handle);
	}

	void Model::clear_removed_entities(Vadon::ECS::World& ecs_world)
	{
		if (m_entity_remove_list.empty() == true)
		{
			// Nothing to do
			return;
		}

		// First remove entities that already have an ancestor in the list
		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		{
			size_t current_index = 0;
			while (current_index < m_entity_remove_list.size())
			{
				bool removed = false;
				const Vadon::ECS::EntityHandle current_entity = m_entity_remove_list[current_index];
				Vadon::ECS::EntityHandle parent_entity = entity_manager.get_entity_parent(current_entity);
				while (parent_entity.is_valid() == true)
				{
					if (std::find(m_entity_remove_list.begin(), m_entity_remove_list.end(), parent_entity) != m_entity_remove_list.end())
					{
						// Swap and pop with end
						removed = true;
						m_entity_remove_list[current_index] = m_entity_remove_list.back();
						m_entity_remove_list.pop_back();
						break;
					}

					parent_entity = entity_manager.get_entity_parent(parent_entity);
				}

				if (removed == false)
				{
					++current_index;
				}
			}
		}

		// Now remove each entity
		for (Vadon::ECS::EntityHandle current_entity : m_entity_remove_list)
		{
			ecs_world.remove_entity(current_entity);
		}

		// Clear the list
		m_entity_remove_list.clear();
	}

	void Model::update_player(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto player_query = component_manager.run_component_query<Player&, Velocity2D*, Health*, Weapon*>();
		for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
		{
			auto player_tuple = player_it.get_tuple();
			Player& player_component = std::get<Player&>(player_tuple);

			if (player_component.damage_timer > 0.0f)
			{
				player_component.damage_timer = std::max(0.0f, player_component.damage_timer - delta_time);
			}

			Velocity2D* player_velocity = std::get<Velocity2D*>(player_tuple);
			Vadon::Utilities::Vector2 facing = Vadon::Utilities::Vector2_Zero;
			if (player_velocity != nullptr)
			{
				// TODO: use acceleration!
				player_velocity->velocity = player_component.input.move_dir * player_velocity->top_speed;
				if (player_component.input.move_dir != Vadon::Utilities::Vector2_Zero)
				{
					facing = Vadon::Utilities::normalize(player_component.input.move_dir);
				}
			}

			Weapon* player_weapon = std::get<Weapon*>(player_tuple);
			if (player_weapon != nullptr)
			{
				player_weapon->active = player_component.input.fire;
				// Only update facing if it changed
				if (facing != Vadon::Utilities::Vector2_Zero)
				{
					player_weapon->aim_direction = facing;
				}
			}
		}
	}

	void Model::update_enemies(Vadon::ECS::World& ecs_world, float /*delta_time*/)
	{
		// FIXME: instead of having to do a query, we should just cache the player entity handle!
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Utilities::Vector2 player_position = Vadon::Utilities::Vector2_Zero;

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

		auto enemy_query = component_manager.run_component_query<Enemy&, Transform2D&, Velocity2D&>();
		for (auto enemy_it = enemy_query.get_iterator(); enemy_it.is_valid() == true; enemy_it.next())
		{
			auto enemy_tuple = enemy_it.get_tuple();
			const Transform2D& enemy_transform = std::get<Transform2D&>(enemy_tuple);
			Velocity2D& enemy_velocity = std::get<Velocity2D&>(enemy_tuple);

			const Vadon::Utilities::Vector2 enemy_to_player = player_position - enemy_transform.position;
			if (Vadon::Utilities::dot(enemy_to_player, enemy_to_player) > 0.01f)
			{
				enemy_velocity.velocity = Vadon::Utilities::normalize(enemy_to_player) * enemy_velocity.top_speed;
			}
			else
			{
				enemy_velocity.velocity = Vadon::Utilities::Vector2_Zero;
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
			Vadon::Utilities::Vector2 position;
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
			else if (component_manager.has_component<Projectile>(collision_it.get_entity()) == true)
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

					Projectile* projectile = component_manager.get_component<Projectile>(other_collision.entity);
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
				deferred_remove_entity(health_it.get_entity());
			}
		}
	}

	void Model::update_spawners(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Scene::SceneSystem& scene_system = m_engine_core.get_system<Vadon::Scene::SceneSystem>();

		// FIXME: instead of having to do a query, we should just cache the player entity handle!
		Vadon::Utilities::Vector2 player_position = Vadon::Utilities::Vector2_Zero;

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

		struct CreateEnemyData
		{
			Vadon::Utilities::Vector2 position;
			Vadon::Scene::SceneHandle prefab;
		};
		std::vector<CreateEnemyData> deferred_spawned_enemies;

		// NOTE: currently all spawners just keep spawning once the delay counts down
		// Need to implement limits on enemy numbers and a priority system
		// Can put all spawners that timed out into a FIFO list
		auto spawner_query = component_manager.run_component_query<Spawner&>();
		for (auto spawner_it = spawner_query.get_iterator(); spawner_it.is_valid() == true; spawner_it.next())
		{
			auto spawner_tuple = spawner_it.get_tuple();
			Spawner& current_spawner = std::get<Spawner&>(spawner_tuple);

			if (current_spawner.activation_delay > 0.0f)
			{
				current_spawner.activation_delay -= delta_time;
			}
			else
			{
				current_spawner.spawn_timer -= delta_time;
				if (current_spawner.spawn_timer <= 0.0f)
				{
					current_spawner.spawn_timer = std::max(current_spawner.spawn_timer + current_spawner.min_spawn_delay, 0.0f);

					// TODO: make this editable!
					constexpr float spawn_radius = 350.0f;

					for (int32_t spawned_enemy_index = 0; spawned_enemy_index < current_spawner.current_spawn_count; ++spawned_enemy_index)
					{
						CreateEnemyData& new_enemy_data = deferred_spawned_enemies.emplace_back();

						const float rand_angle = m_enemy_dist(m_random_engine);

						// Spawn relative to player
						new_enemy_data.position = player_position + Vadon::Utilities::Vector2(std::cosf(rand_angle), std::sinf(rand_angle)) * spawn_radius;

						new_enemy_data.prefab = current_spawner.enemy_prefab;
					}
				}

				if (current_spawner.current_level < current_spawner.max_level)
				{
					current_spawner.level_up_timer -= delta_time;
					if (current_spawner.level_up_timer <= 0.0f)
					{
						++current_spawner.current_level;
						current_spawner.level_up_timer = std::max(current_spawner.level_up_timer + current_spawner.level_up_delay, 0.0f);
					}
				}
			}
		}

		for (const CreateEnemyData& current_enemy_data : deferred_spawned_enemies)
		{
			const Vadon::ECS::EntityHandle spawned_enemy = scene_system.instantiate_scene(current_enemy_data.prefab, ecs_world);

			// FIXME: make a parent entity for enemies?
			// Could be the enemy subsystem
			ecs_world.get_entity_manager().add_child_entity(m_level_root, spawned_enemy);

			Transform2D* transform_component = component_manager.get_component<Transform2D>(spawned_enemy);

			if (transform_component != nullptr)
			{
				// Spawn relative to player
				transform_component->position = current_enemy_data.position;
			}

			// Init health
			// FIXME: could use event for this!
			Health* health_component = component_manager.get_component<Health>(spawned_enemy);
			if (health_component != nullptr)
			{
				health_component->current_health = health_component->max_health;
			}
		}
	}

	void Model::update_weapons(Vadon::ECS::World& ecs_world, float delta_time)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		Vadon::Scene::SceneSystem& scene_system = m_engine_core.get_system<Vadon::Scene::SceneSystem>();

		// Have to defer creating the projectiles, because we can't add to the ECS while iterating in it
		// FIXME: implement deferred operations in ECS?
		struct CreateProjectileData
		{
			Vadon::Utilities::Vector2 position;
			Vadon::Utilities::Vector2 aim_direction;
			Vadon::Scene::SceneHandle prefab;
		};
		std::vector<CreateProjectileData> deferred_projectiles;

		auto weapon_query = component_manager.run_component_query<Weapon&, Transform2D*>();
		for (auto weapon_it = weapon_query.get_iterator(); weapon_it.is_valid() == true; weapon_it.next())
		{
			auto weapon_tuple = weapon_it.get_tuple();
			Weapon& current_weapon = std::get<Weapon&>(weapon_tuple);
			if (current_weapon.firing_timer > 0.0f)
			{
				current_weapon.firing_timer -= delta_time;
			}

			if ((current_weapon.active == true) && (current_weapon.firing_timer <= 0.0f))
			{
				CreateProjectileData& new_projectile_data = deferred_projectiles.emplace_back();

				const Transform2D* weapon_transform = std::get<Transform2D*>(weapon_tuple);
				if (weapon_transform != nullptr)
				{
					new_projectile_data.position = weapon_transform->position;
				}
				else
				{
					new_projectile_data.position = Vadon::Utilities::Vector2_Zero;
				}

				new_projectile_data.aim_direction = current_weapon.aim_direction;
				new_projectile_data.prefab = current_weapon.projectile_prefab;

				// Reset firing timer
				current_weapon.firing_timer += 60.0f / std::max(current_weapon.rate_of_fire, 0.001f);
			}
		}

		for (const CreateProjectileData& current_projectile_data : deferred_projectiles)
		{
			// Spawn projectile
			// FIXME: at the moment we make an entity for every projectile
			// Could use a special "pool" Entity that contains projectile instances
			const Vadon::ECS::EntityHandle spawned_projectile = scene_system.instantiate_scene(current_projectile_data.prefab, ecs_world);

			// FIXME: make a parent entity for projectiles?
			// Could be the projectile subsystem
			ecs_world.get_entity_manager().add_child_entity(m_level_root, spawned_projectile);

			auto projectile_tuple = component_manager.get_component_tuple<Transform2D, Velocity2D, Projectile>(spawned_projectile);

			// Set the initial position
			Transform2D* projectile_transform = std::get<Transform2D*>(projectile_tuple);
			if (projectile_transform != nullptr)
			{
				projectile_transform->position = current_projectile_data.position;
			}

			// Set velocity and lifetime
			Velocity2D* projectile_velocity = std::get<Velocity2D*>(projectile_tuple);
			Projectile* projectile_component = std::get<Projectile*>(projectile_tuple);

			projectile_component->remaining_lifetime = projectile_component->range / projectile_velocity->top_speed;
			projectile_velocity->velocity = current_projectile_data.aim_direction * projectile_velocity->top_speed;
		}
	}

	void Model::update_projectiles(Vadon::ECS::World& ecs_world, float delta_time)
	{
		// Check if any projectiles timed out
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto projectile_query = component_manager.run_component_query<Projectile&>();
		for (auto projectile_it = projectile_query.get_iterator(); projectile_it.is_valid() == true; projectile_it.next())
		{
			auto projectile_tuple = projectile_it.get_tuple();
			Projectile& current_projectile = std::get<Projectile&>(projectile_tuple);
			current_projectile.remaining_lifetime -= delta_time;

			if (current_projectile.remaining_lifetime <= 0.0f)
			{
				// TODO: instead of removing, we should just set a flag and use the ECS as a "pool"
				// Whenever we spawn a projectile again, we can just "resurrect" a projectile that was already used
				deferred_remove_entity(projectile_it.get_entity());
			}
		}
	}
}