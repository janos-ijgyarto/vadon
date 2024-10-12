#include <VadonDemo/Model/Model.hpp>

#include <VadonDemo/Model/Component.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>
#include <Vadon/Scene/SceneSystem.hpp>

#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

#include <numbers>
#include <random>

namespace VadonDemo::Model
{
	namespace
	{
		enum class PacketType : uint32_t
		{
			CREATE_ENEMY_PREFAB,
			CREATE_ENEMY,
			BULK_UPDATE_ENEMIES,
			DESTROY_ENEMY,
			CREATE_PROJECTILE_PREFAB,
			CREATE_PROJECTILE,
			BULK_UPDATE_PROJECTILES,
			DESTROY_PROJECTILE,
			CREATE_GENERIC,
			MODIFY_GENERIC,
			UPDATE_GENERIC,
			DESTROY_GENERIC
		};
		
		struct CreateEnemyPrefab
		{
			int32_t type;
			Vadon::Utilities::Vector3 color;
			float z_order;
			float scale;
		};

		struct CreateEnemy
		{
			Vadon::Utilities::Vector2 position;
			int32_t prefab;
		};

		// NOTE: passed in the same order as object pool contents
		struct UpdateEnemy
		{
			Vadon::Utilities::Vector2 position;
		};

		struct DestroyEnemy
		{
			int32_t index;
		};

		struct CreateProjectilePrefab
		{
			int32_t type;
			Vadon::Utilities::Vector3 color;
			float z_order;
			float scale;
		};

		struct CreateProjectile
		{
			int32_t buffer;
			Vadon::Utilities::Vector2 position;
			int32_t prefab;
		};

		// NOTE: passed in the same order as object pool contents
		struct UpdateProjectile
		{
			Vadon::Utilities::Vector2 position;
		};

		struct DestroyProjectile
		{
			int32_t buffer;
			int32_t offset;
		};

		struct CreateGeneric
		{
			int32_t handle;
		};

		struct ModifyGeneric
		{
			int32_t handle;
			int32_t type;
			Vadon::Utilities::Vector3 color;
			float z_order;
		};

		struct UpdateGeneric
		{
			int32_t handle;
			Vadon::Utilities::Vector2 position;
			float scale;
		};

		struct DestroyGeneric
		{
			int32_t handle;
		};

		struct EnemyData
		{
			int32_t handle;
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 velocity = Vadon::Utilities::Vector2_Zero;
			float speed = 1.0f; // FIXME: used reference to shared data!
			float health = 0.0f;
		};

		struct EnemyPool
		{
			std::vector<EnemyData> enemies;

			std::vector<size_t> sparse_set;
			std::vector<size_t> free_list;

			int32_t prefab_counter = 0;

			int32_t add_enemy(const EnemyData& new_enemy)
			{
				const size_t handle_index = free_list.empty() == false ? free_list.back() : sparse_set.size();
				const size_t dense_index = enemies.size();
				if (free_list.empty() == false)
				{
					free_list.pop_back();
					sparse_set[handle_index] = dense_index;
				}
				else
				{
					// Add to end of sparse set
					sparse_set.push_back(dense_index);
				}

				enemies.push_back(new_enemy);
				enemies.back().handle = static_cast<int32_t>(handle_index);

				return static_cast<int32_t>(handle_index);
			}

			EnemyData& get_enemy(int32_t handle)
			{
				return enemies[sparse_set[handle]];
			}

			void remove_enemy(int32_t handle)
			{
				free_list.push_back(handle);

				const size_t dense_index = sparse_set[handle];
				if (dense_index != (enemies.size() - 1))
				{
					sparse_set[enemies.back().handle] = dense_index;
					enemies[dense_index] = enemies.back();
				}
				enemies.pop_back();
			}
		};

		struct EnemyRenderObject
		{
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;
			int32_t prefab = -1;
		};

		struct EnemyRenderPrefab
		{
			Vadon::Render::Canvas::ItemHandle canvas_item;
			Vadon::Utilities::DataRange batch_range;
		};

		struct EnemyRenderPool
		{
			Vadon::Render::Canvas::BatchHandle enemy_canvas_batch;
			std::vector<EnemyRenderPrefab> prefabs;
			int32_t prefab_count = 0;

			std::vector<EnemyRenderObject> objects;
		};

		struct ProjectileData
		{
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 velocity = Vadon::Utilities::Vector2_Zero;
			float remaining_lifetime = -1.0f;

			bool is_active() const { return remaining_lifetime > 0.0f; }
		};

		struct ProjectileRingBuffer
		{
			static constexpr size_t c_buffer_capacity = 1024;
			std::array<ProjectileData, c_buffer_capacity> projectiles;
			size_t active_count = 0;

			void add_projectile(const ProjectileData& projectile_data)
			{
				// TODO: assert to make sure we can't add more than the capacity!
				projectiles[active_count] = projectile_data;
				++active_count;
			}

			void projectile_expired(size_t index)
			{
				projectiles[index].remaining_lifetime = -1.0f;
			}

			bool is_free() const { return active_count == 0; }
			bool is_full() const { return active_count >= c_buffer_capacity; }
		};

		struct ProjectilePool
		{
			std::vector<std::unique_ptr<ProjectileRingBuffer>> ring_buffers;
			size_t active_buffer_index = 0;

			CreateProjectile add_projectile(const ProjectileData& projectile_data, int32_t prefab)
			{
				ensure_available_buffer();

				const CreateProjectile projectile_event{
					.buffer = static_cast<int32_t>(active_buffer_index),
					.position = projectile_data.position,
					.prefab = prefab
				};

				ProjectileRingBuffer& active_buffer = *ring_buffers[active_buffer_index];
				active_buffer.add_projectile(projectile_data);

				if (active_buffer.is_full() == true)
				{
					active_buffer_index = ring_buffers.size();
				}

				return projectile_event;
			}

			void ensure_available_buffer()
			{
				if (active_buffer_index < ring_buffers.size())
				{
					return;
				}

				for (size_t current_buffer_index = 0; current_buffer_index < ring_buffers.size(); ++current_buffer_index)
				{
					if (ring_buffers[current_buffer_index]->is_free() == true)
					{
						active_buffer_index = current_buffer_index;
						return;
					}
				}

				active_buffer_index = ring_buffers.size();
				ring_buffers.emplace_back(new ProjectileRingBuffer());
			}
		};

		struct ProjectileRenderObject
		{
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;
			int32_t prefab = -1;

			bool is_active() const { return prefab >= 0; }
		};

		struct ProjectileRenderPrefab
		{
			Vadon::Render::Canvas::ItemHandle canvas_item;
			Vadon::Utilities::DataRange batch_range;
		};

		struct RenderProjectileRingBuffer
		{
			static constexpr size_t c_buffer_capacity = 1024;
			std::array<ProjectileRenderObject, c_buffer_capacity> projectiles;
			int32_t active_count = 0;

			bool is_empty() const { return projectiles.front().is_active() == false; }
		};

		struct ProjectileRenderPool
		{
			Vadon::Render::Canvas::BatchHandle projectile_canvas_batch;
			std::vector<ProjectileRenderPrefab> prefabs;
			int32_t prefab_count = 0;

			std::vector<std::unique_ptr<RenderProjectileRingBuffer>> ring_buffers;

			void update(std::span<UpdateProjectile> projectile_data)
			{
				// NOTE: by this stage we expect we already added/removed all projectiles for this frame
				size_t current_buffer_index = 0;
				size_t data_offset = 0;
				while (current_buffer_index < ring_buffers.size())
				{
					RenderProjectileRingBuffer& current_buffer = *ring_buffers[current_buffer_index];					
					if (current_buffer.is_empty() == false)
					{
						for (ProjectileRenderObject& current_projectile : current_buffer.projectiles)
						{
							if (current_projectile.is_active() == false)
							{
								break;
							}

							const UpdateProjectile& update_data = projectile_data[data_offset];
							current_projectile.prev_position = current_projectile.position;
							current_projectile.position = update_data.position;
							++data_offset;
						}
					}
					++current_buffer_index;

				}
			}
		};

		struct GenericRenderObject
		{
			Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
			Vadon::Utilities::Vector2 prev_position = Vadon::Utilities::Vector2_Zero;
			float scale = 1.0f;

			Vadon::Render::Canvas::ItemHandle canvas_item;
		};

		struct GenericRenderPool
		{
			std::vector<GenericRenderObject> objects;
			std::vector<size_t> free_list;
			int32_t active_object_count = 0;

			int32_t register_object()
			{
				int32_t new_handle = -1;
				if (free_list.empty() == false)
				{
					new_handle = static_cast<int32_t>(free_list.back());
					free_list.pop_back();
				}
				else
				{
					new_handle = active_object_count;
				}

				++active_object_count;
				return new_handle;
			}

			void unregister_object(int32_t handle)
			{
				free_list.push_back(handle);
				--active_object_count;
			}

			void add_object(int32_t handle)
			{
				if (handle >= static_cast<int32_t>(objects.size()))
				{
					objects.emplace_back();
				}
			}
		};

		struct SpawnData
		{
			Vadon::ECS::EntityHandle prefab_entity;
			float spawn_delay = 0.0f;
		};

		Vadon::Utilities::Vector2 rotate_2d_vector(const Vadon::Utilities::Vector2& vector, float angle)
		{
			const float sin_angle = std::sinf(angle);
			const float cos_angle = std::cosf(angle);

			return Vadon::Utilities::Vector2(cos_angle * vector.x - sin_angle * vector.y, sin_angle * vector.x + cos_angle * vector.y);
		}
	}

	struct Model::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Vadon::Render::Canvas::LayerHandle m_canvas_layer; // FIXME: should not embed this in model!

		float m_interpolation_weight = 1.0f;

		EnemyPool m_enemy_pool;
		ProjectilePool m_projectile_pool;

		EnemyRenderPool m_enemy_render_pool;
		ProjectileRenderPool m_projectile_render_pool;
		GenericRenderPool m_generic_render_pool;

		Vadon::ECS::EntityHandle m_player_entity;
		Vadon::ECS::EntityHandle m_map_entity;

		Vadon::Utilities::PacketQueue m_event_queue;
 
		std::mt19937 m_random_engine;
		std::uniform_real_distribution<float> m_enemy_dist;

		Internal(Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
			, m_random_engine(std::random_device{}())
			, m_enemy_dist(0.0f, 2 * std::numbers::pi_v<float>)
		{
		}

		bool initialize()
		{
			Transform2D::register_component();
			Velocity2D::register_component();
			CanvasComponent::register_component();
			Health::register_component();
			Player::register_component();
			Weapon::register_component();
			Projectile::register_component();
			Enemy::register_component();
			Map::register_component();
			Spawner::register_component();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			m_canvas_layer = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{});

			return true;
		}

		bool init_simulation(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			// Validate player and map
			// TODO: alterantively, could make it a required setting in the scene?
			// Some kind of special scene which spawns a player and map?
			{
				auto player_query = component_manager.run_component_query<Player&>();
				for (auto player_it = player_query.get_iterator(); player_it.is_valid() == true; player_it.next())
				{
					if (m_player_entity.is_valid() == true)
					{
						// TODO: error!
						return false;
					}

					m_player_entity = player_it.get_entity();
					auto player_components = component_manager.get_component_tuple<Transform2D, Velocity2D, Health, Weapon>(m_player_entity);
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
					if (std::get<Health*>(player_components) == nullptr)
					{
						// TODO: error!
						return false;
					}
					Weapon* player_weapon = std::get<Weapon*>(player_components);
					if (player_weapon == nullptr)
					{
						// TODO: error!
						return false;
					}

					if (load_weapon(ecs_world, *player_weapon) == false)
					{
						// TODO: error!
						return false;
					}
				}

				if (m_player_entity.is_valid() == false)
				{
					// TODO: error!
					return false;
				}

				CanvasComponent* player_canvas_component = component_manager.get_component<CanvasComponent>(m_player_entity);
				if (player_canvas_component != nullptr)
				{
					player_canvas_component->render_handle = m_generic_render_pool.register_object();

					CreateGeneric* create_player_render_obj = m_event_queue.allocate_object<CreateGeneric>(Vadon::Utilities::to_integral(PacketType::CREATE_GENERIC));
					create_player_render_obj->handle = player_canvas_component->render_handle;

					ModifyGeneric* modify_player_render_obj = m_event_queue.allocate_object<ModifyGeneric>(Vadon::Utilities::to_integral(PacketType::MODIFY_GENERIC));
					modify_player_render_obj->handle = player_canvas_component->render_handle;
					modify_player_render_obj->type = player_canvas_component->type;
					modify_player_render_obj->color = player_canvas_component->color;
					modify_player_render_obj->z_order = player_canvas_component->z_order;
				}				
			}

			{
				auto map_query = component_manager.run_component_query<Map&>();

				for (auto map_it = map_query.get_iterator(); map_it.is_valid() == true; map_it.next())
				{
					if (m_map_entity.is_valid() == true)
					{
						// TODO: error!
						return false;
					}

					m_map_entity = map_it.get_entity();

					auto map_components = component_manager.get_component_tuple<Transform2D, Map>(m_map_entity);
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

				if (m_map_entity.is_valid() == false)
				{
					// TODO: error!
					return false;
				}

				CanvasComponent* map_canvas_component = component_manager.get_component<CanvasComponent>(m_map_entity);
				if (map_canvas_component != nullptr)
				{
					map_canvas_component->render_handle = m_generic_render_pool.register_object();

					CreateGeneric* create_map_render_obj = m_event_queue.allocate_object<CreateGeneric>(Vadon::Utilities::to_integral(PacketType::CREATE_GENERIC));
					create_map_render_obj->handle = map_canvas_component->render_handle;

					ModifyGeneric* modify_map_render_obj = m_event_queue.allocate_object<ModifyGeneric>(Vadon::Utilities::to_integral(PacketType::MODIFY_GENERIC));
					modify_map_render_obj->handle = map_canvas_component->render_handle;
					modify_map_render_obj->type = map_canvas_component->type;
					modify_map_render_obj->color = map_canvas_component->color;
					modify_map_render_obj->z_order = map_canvas_component->z_order;
				}
			}

			{
				auto spawner_query = component_manager.run_component_query<Spawner&>();
				bool spawner_loaded = false;

				for (auto spawner_it = spawner_query.get_iterator(); spawner_it.is_valid() == true; spawner_it.next())
				{
					auto spawner_tuple = spawner_it.get_tuple();
					Spawner& current_spawner = std::get<Spawner&>(spawner_tuple);

					if (load_spawner_prefab(ecs_world, current_spawner) == false)
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

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			m_enemy_render_pool.enemy_canvas_batch = canvas_system.create_batch();
			m_projectile_render_pool.projectile_canvas_batch = canvas_system.create_batch();
	
			// TODO2: implement system where presence of component will prompt processing of sub-scene with specific logic
			// For example, creating renderable with multiple pieces using Entities, then "condensing" at runtime

			// TODO: logic to unload render resources! (to clean up on shutdown)

			return true;
		}

		bool load_weapon(Vadon::ECS::World& ecs_world, Weapon& weapon_component)
		{
			if (weapon_component.projectile_prefab.empty() == true)
			{
				// TODO: error!
				return false;
			}

			Vadon::Scene::ResourceID projectile_prefab_id;
			if (projectile_prefab_id.from_base64_string(weapon_component.projectile_prefab) == false)
			{
				// TODO: error!
				return false;
			}

			if (projectile_prefab_id.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			Vadon::Scene::ResourceHandle projectile_prefab_scene = resource_system.find_resource(projectile_prefab_id);

			if (projectile_prefab_scene.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			// FIXME: check to make sure we haven't already loaded this?
			Vadon::Scene::SceneSystem& scene_system = m_engine_core.get_system<Vadon::Scene::SceneSystem>();
			weapon_component.projectile_prefab_entity = scene_system.instantiate_scene(projectile_prefab_scene, ecs_world);

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto projectile_prefab_components = component_manager.get_component_tuple<Transform2D, Velocity2D, CanvasComponent, Projectile>(weapon_component.projectile_prefab_entity);

			Transform2D* transform = std::get<Transform2D*>(projectile_prefab_components);
			if (transform == nullptr)
			{
				// TODO: error!
				return false;
			}
			if (std::get<Velocity2D*>(projectile_prefab_components) == nullptr)
			{
				// TODO: error!
				return false;
			}
			CanvasComponent* canvas_component = std::get<CanvasComponent*>(projectile_prefab_components);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return false;
			}

			if (std::get<Projectile*>(projectile_prefab_components) == nullptr)
			{
				// TODO: error!
				return false;
			}

			CreateProjectilePrefab* create_proj_prefab = m_event_queue.allocate_object<CreateProjectilePrefab>(Vadon::Utilities::to_integral(PacketType::CREATE_PROJECTILE_PREFAB));
			create_proj_prefab->type = canvas_component->type;
			create_proj_prefab->color = canvas_component->color;
			create_proj_prefab->z_order = canvas_component->z_order;
			create_proj_prefab->scale = transform->scale;

			return true;
		}

		bool load_spawner_prefab(Vadon::ECS::World& ecs_world, Spawner& spawner)
		{
			if (spawner.enemy_prefab.empty() == true)
			{
				// TODO: error!
				return false;
			}

			Vadon::Scene::ResourceID enemy_prefab_id;
			if (enemy_prefab_id.from_base64_string(spawner.enemy_prefab) == false)
			{
				// TODO: error!
				return false;
			}

			if (enemy_prefab_id.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			Vadon::Scene::ResourceHandle enemy_prefab_scene = resource_system.find_resource(enemy_prefab_id);

			if (enemy_prefab_scene.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			// FIXME: check to make sure we haven't already loaded this?
			Vadon::Scene::SceneSystem& scene_system = m_engine_core.get_system<Vadon::Scene::SceneSystem>();
			spawner.enemy_prefab_entity = scene_system.instantiate_scene(enemy_prefab_scene, ecs_world);

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto enemy_prefab_components = component_manager.get_component_tuple<Transform2D, Velocity2D, CanvasComponent, Health, Enemy>(spawner.enemy_prefab_entity);

			Transform2D* transform = std::get<Transform2D*>(enemy_prefab_components);
			if (transform == nullptr)
			{
				// TODO: error!
				return false;
			}
			if (std::get<Velocity2D*>(enemy_prefab_components) == nullptr)
			{
				// TODO: error!
				return false;
			}

			CanvasComponent* canvas_component = std::get<CanvasComponent*>(enemy_prefab_components);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return false;
			}
			if (std::get<Health*>(enemy_prefab_components) == nullptr)
			{
				// TODO: error!
				return false;
			}
			Enemy* enemy_component = std::get<Enemy*>(enemy_prefab_components);
			if (enemy_component == nullptr)
			{
				// TODO: error!
				return false;
			}

			// FIXME: this means Enemy component can only ever be used with spawner!
			enemy_component->prefab = m_enemy_pool.prefab_counter;
			++m_enemy_pool.prefab_counter;

			CreateEnemyPrefab* create_enemy_prefab = m_event_queue.allocate_object<CreateEnemyPrefab>(Vadon::Utilities::to_integral(PacketType::CREATE_ENEMY_PREFAB));
			create_enemy_prefab->type = canvas_component->type;
			create_enemy_prefab->color = canvas_component->color;
			create_enemy_prefab->z_order = canvas_component->z_order;
			create_enemy_prefab->scale = transform->scale;

			return true;
		}

		void update_simulation(Vadon::ECS::World& ecs_world, float delta_time)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto player_components = component_manager.get_component_tuple<Transform2D, Velocity2D, Player>(m_player_entity);

			Transform2D* player_transform = std::get<Transform2D*>(player_components);
			Velocity2D* player_velocity = std::get<Velocity2D*>(player_components);
			Player* player_component = std::get<Player*>(player_components);

			// TODO: use acceleration!
			player_velocity->velocity = player_component->input.move_dir * player_velocity->top_speed;
			player_transform->position += player_velocity->velocity * delta_time;

			if (player_component->input.move_dir != Vadon::Utilities::Vector2_Zero)
			{
				player_component->facing = Vadon::Utilities::normalize(player_component->input.move_dir);
			}

			for(EnemyData& current_enemy : m_enemy_pool.enemies)
			{
				const Vadon::Utilities::Vector2 enemy_to_player = player_transform->position - current_enemy.position;
				if (Vadon::Utilities::dot(enemy_to_player, enemy_to_player) > 0.01f)
				{
					current_enemy.velocity = Vadon::Utilities::normalize(enemy_to_player) * current_enemy.speed;
				}
				else
				{
					current_enemy.velocity = Vadon::Utilities::Vector2_Zero;
				}
				current_enemy.position += current_enemy.velocity * delta_time;

				// TODO: enemies dying!
			}

			update_projectiles(ecs_world, delta_time);

			// Update spawners
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

						auto enemy_prefab_components = component_manager.get_component_tuple<Velocity2D, Health, Enemy>(current_spawner.enemy_prefab_entity);
						const Velocity2D* enemy_velocity = std::get<Velocity2D*>(enemy_prefab_components);
						const Health* enemy_health = std::get<Health*>(enemy_prefab_components);
						const Enemy* enemy_component = std::get<Enemy*>(enemy_prefab_components);

						constexpr float spawn_radius = 350.0f;

						for (int32_t spawned_enemy_index = 0; spawned_enemy_index < current_spawner.current_spawn_count; ++spawned_enemy_index)
						{
							const float rand_angle = m_enemy_dist(m_random_engine);

							const EnemyData new_enemy{
								.position = Vadon::Utilities::Vector2(std::cosf(rand_angle), std::sinf(rand_angle)) * spawn_radius,
								.velocity = Vadon::Utilities::Vector2_Zero,
								.speed = enemy_velocity->top_speed,
								.health = enemy_health->max_health
							};

							m_enemy_pool.add_enemy(new_enemy);
							const CreateEnemy create_enemy_data{
								.position = new_enemy.position,
								.prefab = enemy_component->prefab
							};
							m_event_queue.write_object(Vadon::Utilities::to_integral(PacketType::CREATE_ENEMY), create_enemy_data);
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
		}

		void update_projectiles(Vadon::ECS::World& ecs_world, float delta_time)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto player_components = component_manager.get_component_tuple<Transform2D, Player, Weapon>(m_player_entity);

			const Transform2D* player_transform = std::get<Transform2D*>(player_components);
			const Player* player_component = std::get<Player*>(player_components);
			Weapon* player_weapon = std::get<Weapon*>(player_components);

			player_weapon->firing_timer = std::max(player_weapon->firing_timer - delta_time, 0.0f);

			if ((player_weapon->firing_timer <= 0.0f) && (player_component->input.fire == true))
			{
				// Reset timer and fire projectile
				player_weapon->firing_timer = 60.0f / player_weapon->rate_of_fire;

				auto projectile_components = component_manager.get_component_tuple<Velocity2D, Projectile>(player_weapon->projectile_prefab_entity);

				const Velocity2D* projectile_velocity = std::get<Velocity2D*>(projectile_components);
				const Projectile* projectile_component = std::get<Projectile*>(projectile_components);

				const ProjectileData new_projectile{
					.position = player_transform->position,
					.velocity = player_component->facing * projectile_velocity->top_speed,
					.remaining_lifetime = projectile_component->range / projectile_velocity->top_speed
				};

				// FIXME: create lookup for projectile prefabs!
				const CreateProjectile proj_event_data = m_projectile_pool.add_projectile(new_projectile, 0);
				m_event_queue.write_object(Vadon::Utilities::to_integral(PacketType::CREATE_PROJECTILE), proj_event_data);
			}

			// FIXME: could separate the queues and first process creation/destruction
			// This would allow queuing the updates in the same loop
			for (size_t current_buffer_index = 0; current_buffer_index < m_projectile_pool.ring_buffers.size(); ++current_buffer_index)
			{
				auto& current_buffer = m_projectile_pool.ring_buffers[current_buffer_index];
				// Update projectiles 
				if (current_buffer->is_free() == true)
				{
					return;
				}

				size_t current_index = 0;
				while (current_index < current_buffer->active_count)
				{
					ProjectileData& current_projectile = current_buffer->projectiles[current_index];
					if (current_projectile.is_active() == true)
					{
						current_projectile.position += delta_time * current_projectile.velocity;
						current_projectile.remaining_lifetime -= delta_time;
					}
					if (current_projectile.is_active() == false)
					{
						m_event_queue.write_object(Vadon::Utilities::to_integral(PacketType::DESTROY_PROJECTILE), DestroyProjectile{ .buffer = static_cast<int32_t>(current_buffer_index), .offset = static_cast<int32_t>(current_index) });
						if (current_index < current_buffer->active_count - 1)
						{
							current_buffer->projectiles[current_index] = current_buffer->projectiles[current_buffer->active_count - 1];
						}
						--current_buffer->active_count;
						continue;
					}
					++current_index;
				}
			}
		}

		void render_sync(Vadon::ECS::World& ecs_world)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto render_query = component_manager.run_component_query<CanvasComponent&, Transform2D*>();
			auto render_it = render_query.get_iterator();

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();

				Transform2D* transform_component = std::get<Transform2D*>(current_component_tuple);
				CanvasComponent& canvas_component = std::get<CanvasComponent&>(current_component_tuple);

				if (canvas_component.render_handle < 0)
				{
					internal_update_canvas_item(canvas_component);
					assert(canvas_component.render_handle >= 0);
				}

				GenericRenderObject& render_object = m_generic_render_pool.objects[canvas_component.render_handle];
				float scale = 1.0f;
				if (transform_component != nullptr)
				{
					render_object.position = transform_component->position;
					scale = transform_component->scale;
				}
				else
				{
					render_object.position = Vadon::Utilities::Vector2_Zero;
				}

				canvas_system.set_item_transform(render_object.canvas_item, Vadon::Render::Canvas::Transform{ .position = render_object.position, .scale = scale });
			}

			// TODO: synchronous rendering of projectiles and enemies!
		}

		void update_view_async(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue)
		{
			render_queue.clear();

			if (m_event_queue.is_empty() == false)
			{
				render_queue.append_queue(m_event_queue);
				m_event_queue.clear();
			}

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			auto render_query = component_manager.run_component_query<CanvasComponent&, Transform2D*>();
			auto render_it = render_query.get_iterator();

			for (; render_it.is_valid() == true; render_it.next())
			{
				auto current_entity = render_it.get_entity();
				auto current_component_tuple = render_it.get_tuple();

				Transform2D* transform_component = std::get<Transform2D*>(current_component_tuple);
				CanvasComponent& canvas_component = std::get<CanvasComponent&>(current_component_tuple);

				if (canvas_component.render_handle < 0)
				{
					continue;
				}

				UpdateGeneric* update_data = render_queue.allocate_object<UpdateGeneric>(Vadon::Utilities::to_integral(PacketType::UPDATE_GENERIC));
				update_data->handle = canvas_component.render_handle;

				if (transform_component != nullptr)
				{
					update_data->position = transform_component->position;
					update_data->scale = transform_component->scale;
				}
				else
				{
					update_data->position = Vadon::Utilities::Vector2_Zero;
					update_data->scale = 1.0f;
				}
			}

			if(m_enemy_pool.enemies.empty() == false)
			{
				UpdateEnemy* update_ptr = reinterpret_cast<UpdateEnemy*>(render_queue.allocate_raw_data(Vadon::Utilities::to_integral(PacketType::BULK_UPDATE_ENEMIES), m_enemy_pool.enemies.size() * sizeof(UpdateEnemy)));
				for (const EnemyData& current_object : m_enemy_pool.enemies)
				{
					update_ptr->position = current_object.position;
					++update_ptr;
				}
			}

			{
				size_t total_projectile_count = 0;
				for (const auto& current_buffer : m_projectile_pool.ring_buffers)
				{
					total_projectile_count += current_buffer->active_count;
				}

				if (total_projectile_count > 0)
				{
					UpdateProjectile* update_ptr = reinterpret_cast<UpdateProjectile*>(render_queue.allocate_raw_data(Vadon::Utilities::to_integral(PacketType::BULK_UPDATE_PROJECTILES), total_projectile_count * sizeof(UpdateProjectile)));
					for (const auto& current_buffer : m_projectile_pool.ring_buffers)
					{
						for (size_t current_projectile_index = 0; current_projectile_index < current_buffer->active_count; ++current_projectile_index)
						{
							update_ptr->position = current_buffer->projectiles[current_projectile_index].position;
							++update_ptr;
						}
					}
				}
			}
		}

		void render_async(const Vadon::Utilities::PacketQueue& render_queue)
		{
			// FIXME: properly decouple view from model!
			Vadon::Utilities::PacketQueue::Iterator queue_iterator = render_queue.get_iterator();
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			while(queue_iterator.is_valid() == true)
			{
				const PacketType packet_type = Vadon::Utilities::to_enum<PacketType>(queue_iterator.get_header_id());
				switch (packet_type)
				{
				case PacketType::CREATE_ENEMY_PREFAB:
				{
					const CreateEnemyPrefab* create_prefab = reinterpret_cast<const CreateEnemyPrefab*>(queue_iterator.get_packet_data());

					EnemyRenderPrefab& new_prefab = m_enemy_render_pool.prefabs.emplace_back();

					Vadon::Render::Canvas::ItemInfo new_item_info;
					new_item_info.layer = m_canvas_layer;
					new_item_info.z_order = create_prefab->z_order;

					new_prefab.canvas_item = canvas_system.create_item(new_item_info);

					const int32_t render_type = std::clamp(create_prefab->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));
					new_prefab.batch_range = prepare_canvas_batch(new_prefab.canvas_item, m_enemy_render_pool.enemy_canvas_batch, Vadon::Utilities::to_enum<RenderObjectType>(render_type), create_prefab->color, create_prefab->scale);

					queue_iterator.advance(sizeof(CreateEnemyPrefab));
				}
					break;
				case PacketType::CREATE_ENEMY:
				{
					const CreateEnemy* create_enemy = reinterpret_cast<const CreateEnemy*>(queue_iterator.get_packet_data());

					EnemyRenderObject& render_obj =	m_enemy_render_pool.objects.emplace_back();
					render_obj.position = create_enemy->position;
					render_obj.prev_position = create_enemy->position;
					render_obj.prefab = create_enemy->prefab;

					queue_iterator.advance(sizeof(CreateEnemy));
				}
					break;
				case PacketType::BULK_UPDATE_ENEMIES:
				{
					const UpdateEnemy* update_ptr = reinterpret_cast<const UpdateEnemy*>(queue_iterator.get_packet_data());
					for(EnemyRenderObject& current_object : m_enemy_render_pool.objects)
					{
						current_object.prev_position = current_object.position;
						current_object.position = update_ptr->position;
						++update_ptr;
					}

					queue_iterator.advance(sizeof(UpdateEnemy) * m_enemy_render_pool.objects.size());
				}
					break;
				case PacketType::DESTROY_ENEMY:
				{
					const DestroyEnemy* destroy_data = reinterpret_cast<const DestroyEnemy*>(queue_iterator.get_packet_data());

					// TODO: check to make sure we don't get this message if no objects existed to begin with!
					if (destroy_data->index < m_enemy_render_pool.objects.size() - 1)
					{
						m_enemy_render_pool.objects[destroy_data->index] = m_enemy_render_pool.objects.back();
						m_enemy_render_pool.objects.pop_back();
					}

					queue_iterator.advance(sizeof(DestroyEnemy));
				}
					break;
				case PacketType::CREATE_PROJECTILE_PREFAB:
				{
					const CreateProjectilePrefab* create_prefab = reinterpret_cast<const CreateProjectilePrefab*>(queue_iterator.get_packet_data());

					ProjectileRenderPrefab& new_prefab = m_projectile_render_pool.prefabs.emplace_back();

					Vadon::Render::Canvas::ItemInfo new_item_info;
					new_item_info.layer = m_canvas_layer;
					new_item_info.z_order = create_prefab->z_order;

					new_prefab.canvas_item = canvas_system.create_item(new_item_info);

					const int32_t render_type = std::clamp(create_prefab->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));
					new_prefab.batch_range = prepare_canvas_batch(new_prefab.canvas_item, m_projectile_render_pool.projectile_canvas_batch, Vadon::Utilities::to_enum<RenderObjectType>(render_type), create_prefab->color, create_prefab->scale);

					queue_iterator.advance(sizeof(CreateProjectilePrefab));
				}
				break;
				case PacketType::CREATE_PROJECTILE:
				{
					const CreateProjectile* create_projectile = reinterpret_cast<const CreateProjectile*>(queue_iterator.get_packet_data());

					if (create_projectile->buffer >= static_cast<int32_t>(m_projectile_render_pool.ring_buffers.size()))
					{
						m_projectile_render_pool.ring_buffers.emplace_back(new RenderProjectileRingBuffer);
					}

					RenderProjectileRingBuffer& buffer = *m_projectile_render_pool.ring_buffers[create_projectile->buffer];

					ProjectileRenderObject& render_obj = buffer.projectiles[buffer.active_count];
					render_obj.position = create_projectile->position;
					render_obj.prev_position = create_projectile->position;
					render_obj.prefab = create_projectile->prefab;

					++buffer.active_count;

					queue_iterator.advance(sizeof(CreateProjectile));
				}
					break;
				case PacketType::BULK_UPDATE_PROJECTILES:
				{
					const UpdateProjectile* update_ptr = reinterpret_cast<const UpdateProjectile*>(queue_iterator.get_packet_data());
					size_t total_projectile_count = 0;
					for (auto& current_buffer : m_projectile_render_pool.ring_buffers)
					{
						for (int32_t current_projectile_index = 0; current_projectile_index < current_buffer->active_count; ++current_projectile_index)
						{
							ProjectileRenderObject& current_projectile = current_buffer->projectiles[current_projectile_index];
							current_projectile.prev_position = current_projectile.position;
							current_projectile.position = update_ptr->position;

							++total_projectile_count;
							++update_ptr;
						}
					}

					queue_iterator.advance(sizeof(UpdateProjectile) * total_projectile_count);
				}
					break;
				case PacketType::DESTROY_PROJECTILE:
				{
					const DestroyProjectile* destroy_data = reinterpret_cast<const DestroyProjectile*>(queue_iterator.get_packet_data());

					// TODO: check to make sure we don't get this message if no objects existed to begin with!
					RenderProjectileRingBuffer& buffer = *m_projectile_render_pool.ring_buffers[destroy_data->buffer];
					if (destroy_data->offset < buffer.active_count - 1)
					{
						buffer.projectiles[destroy_data->offset] = buffer.projectiles[buffer.active_count - 1];
					}
					--buffer.active_count;

					queue_iterator.advance(sizeof(DestroyProjectile));
				}
					break;
				case PacketType::CREATE_GENERIC:
				{
					const CreateGeneric* create_generic = reinterpret_cast<const CreateGeneric*>(queue_iterator.get_packet_data());

					m_generic_render_pool.add_object(create_generic->handle);

					GenericRenderObject& render_obj = m_generic_render_pool.objects[create_generic->handle];
					
					Vadon::Render::Canvas::ItemInfo new_item_info;
					new_item_info.layer = m_canvas_layer;

					render_obj.canvas_item = canvas_system.create_item(new_item_info);

					queue_iterator.advance(sizeof(CreateGeneric));
				}
					break;
				case PacketType::MODIFY_GENERIC:
				{
					const ModifyGeneric* modify_generic = reinterpret_cast<const ModifyGeneric*>(queue_iterator.get_packet_data());

					GenericRenderObject& render_obj = m_generic_render_pool.objects[modify_generic->handle];

					const int32_t render_type = std::clamp(modify_generic->type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));
					prepare_canvas_item(render_obj.canvas_item, Vadon::Utilities::to_enum<RenderObjectType>(render_type), modify_generic->color);

					canvas_system.set_item_z_order(render_obj.canvas_item, modify_generic->z_order);

					queue_iterator.advance(sizeof(ModifyGeneric));
				}
					break;
				case PacketType::UPDATE_GENERIC:
				{
					const UpdateGeneric* update_generic = reinterpret_cast<const UpdateGeneric*>(queue_iterator.get_packet_data());

					GenericRenderObject& render_obj = m_generic_render_pool.objects[update_generic->handle];
					render_obj.prev_position = render_obj.position;
					render_obj.position = update_generic->position;
					render_obj.scale = update_generic->scale;

					queue_iterator.advance(sizeof(UpdateGeneric));
				}
					break;
				case PacketType::DESTROY_GENERIC:
				{
					const DestroyGeneric* destroy_generic = reinterpret_cast<const DestroyGeneric*>(queue_iterator.get_packet_data());

					// TODO: make sure handle is within bounds!
					GenericRenderObject& render_obj = m_generic_render_pool.objects[destroy_generic->handle];

					if (render_obj.canvas_item.is_valid() == true)
					{
						canvas_system.remove_item(render_obj.canvas_item);

						render_obj.canvas_item.invalidate();
					}

					queue_iterator.advance(sizeof(DestroyGeneric));
				}
					break;
				}
			}
		}

		void lerp_view_state(float lerp_weight)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			const float neg_lerp_weight = 1.0f - lerp_weight;
			
			// First clear all prefabs
			for (const EnemyRenderPrefab& current_prefab : m_enemy_render_pool.prefabs)
			{
				canvas_system.clear_item(current_prefab.canvas_item);
			}

			for (const ProjectileRenderPrefab& current_prefab : m_projectile_render_pool.prefabs)
			{
				canvas_system.clear_item(current_prefab.canvas_item);
			}

			Vadon::Render::Canvas::BatchDrawCommand batch_command;
			batch_command.batch = m_enemy_render_pool.enemy_canvas_batch;
			for (EnemyRenderObject& enemy_obj : m_enemy_render_pool.objects)
			{
				batch_command.transform.position = (enemy_obj.position * lerp_weight) + (enemy_obj.prev_position * neg_lerp_weight);
				batch_command.transform.scale = 1.0f;

				const EnemyRenderPrefab& prefab = m_enemy_render_pool.prefabs[enemy_obj.prefab];
				batch_command.range = prefab.batch_range;

				canvas_system.add_item_batch_draw(prefab.canvas_item, batch_command);
			}

			batch_command.batch = m_projectile_render_pool.projectile_canvas_batch;
			for (const auto& current_buffer : m_projectile_render_pool.ring_buffers)
			{
				for (int32_t current_projectile_index = 0; current_projectile_index < current_buffer->active_count; ++current_projectile_index)
				{
					const ProjectileRenderObject& current_projectile = current_buffer->projectiles[current_projectile_index];

					batch_command.transform.position = (current_projectile.position * lerp_weight) + (current_projectile.prev_position * neg_lerp_weight);
					batch_command.transform.scale = 1.0f;

					const ProjectileRenderPrefab& prefab = m_projectile_render_pool.prefabs[current_projectile.prefab];
					batch_command.range = prefab.batch_range;

					canvas_system.add_item_batch_draw(prefab.canvas_item, batch_command);
				}
			}

			for (const GenericRenderObject& current_obj : m_generic_render_pool.objects)
			{
				if (current_obj.canvas_item.is_valid() == false)
				{
					continue;
				}

				Vadon::Render::Canvas::Transform new_transform;
				new_transform.position = (current_obj.position * lerp_weight) + (current_obj.prev_position * neg_lerp_weight);
				new_transform.scale = current_obj.scale;

				canvas_system.set_item_transform(current_obj.canvas_item, new_transform);
			}
		}

		Vadon::Utilities::DataRange prepare_canvas_batch(Vadon::Render::Canvas::ItemHandle canvas_item, Vadon::Render::Canvas::BatchHandle canvas_batch, RenderObjectType type, const Vadon::Utilities::Vector3& color, float scale)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.clear_item(canvas_item);

			Vadon::Utilities::DataRange batch_range;
			batch_range.offset = static_cast<int32_t>(canvas_system.get_batch_buffer_size(canvas_batch));
				
			switch (type)
			{
			case RenderObjectType::TRIANGLE:
			{
				Vadon::Render::Canvas::Triangle triangle;
				triangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);

				triangle.point_a.position = { 0, 0.5f * scale };
				triangle.point_b.position = { 0.5f * scale, -0.5f * scale };
				triangle.point_c.position = { -0.5f * scale, -0.5f * scale };

				canvas_system.draw_batch_triangle(canvas_batch, triangle);
				batch_range.count = 1;
			}
			break;
			case RenderObjectType::BOX:
			{
				Vadon::Render::Canvas::Rectangle box_rectangle;
				box_rectangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);
				box_rectangle.dimensions.size = { 1.0f * scale, 1.0f * scale };
				box_rectangle.filled = true;

				canvas_system.draw_batch_rectangle(canvas_batch, box_rectangle);
				batch_range.count = 1;
			}
			break;
			case RenderObjectType::DIAMOND:
			{
				Vadon::Render::Canvas::Triangle diamond_half_triangle;
				diamond_half_triangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);

				diamond_half_triangle.point_a.position = { 0, 1.0f * scale };
				diamond_half_triangle.point_b.position = { 1.0f * scale, 0 };
				diamond_half_triangle.point_c.position = { -1.0f * scale, 0 };

				canvas_system.draw_batch_triangle(canvas_batch, diamond_half_triangle);

				diamond_half_triangle.point_a.position = { -1.0f * scale, 0 };
				diamond_half_triangle.point_b.position = { 1.0f * scale, 0 };
				diamond_half_triangle.point_c.position = { 0, -1.0f * scale };

				canvas_system.draw_batch_triangle(canvas_batch, diamond_half_triangle);
				batch_range.count = 2;
			}
			break;
			}

			return batch_range;
		}

		void prepare_canvas_item(Vadon::Render::Canvas::ItemHandle canvas_item, RenderObjectType type, const Vadon::Utilities::Vector3& color)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.clear_item(canvas_item);

			switch (type)
			{
			case RenderObjectType::TRIANGLE:
			{
				Vadon::Render::Canvas::Triangle triangle;
				triangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);

				triangle.point_a.position = { 0, 0.5f };
				triangle.point_b.position = { 0.5f, -0.5f };
				triangle.point_c.position = { -0.5f, -0.5f };

				canvas_system.draw_item_triangle(canvas_item, triangle);
			}
			break;
			case RenderObjectType::BOX:
			{
				Vadon::Render::Canvas::Rectangle box_rectangle;
				box_rectangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);
				box_rectangle.dimensions.size = { 1.0f, 1.0f };
				box_rectangle.filled = true;

				canvas_system.draw_item_rectangle(canvas_item, box_rectangle);
			}
			break;
			case RenderObjectType::DIAMOND:
			{
				Vadon::Render::Canvas::Triangle diamond_half_triangle;
				diamond_half_triangle.color = Vadon::Render::Canvas::ColorRGBA(color, 1.0f);

				diamond_half_triangle.point_a.position = { 0, 1.0f };
				diamond_half_triangle.point_b.position = { 1.0f, 0 };
				diamond_half_triangle.point_c.position = { -1.0f, 0 };

				canvas_system.draw_item_triangle(canvas_item, diamond_half_triangle);

				diamond_half_triangle.point_a.position = { -1.0f, 0 };
				diamond_half_triangle.point_b.position = { 1.0f, 0 };
				diamond_half_triangle.point_c.position = { 0, -1.0f };

				canvas_system.draw_item_triangle(canvas_item, diamond_half_triangle);
			}
			break;
			}
		}

		void update_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			// NOTE: this function assumes we are in a single-threaded context (i.e editor)
			CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return;
			}

			internal_update_canvas_item(*canvas_component);
		}

		void internal_update_canvas_item(CanvasComponent& canvas_component)
		{
			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			if (canvas_component.render_handle < 0)
			{
				// TODO: create canvas item
				canvas_component.render_handle = m_generic_render_pool.register_object();
				m_generic_render_pool.add_object(canvas_component.render_handle);

				GenericRenderObject& render_obj = m_generic_render_pool.objects[canvas_component.render_handle];

				Vadon::Render::Canvas::ItemInfo new_item_info;
				new_item_info.layer = m_canvas_layer;

				render_obj.canvas_item = canvas_system.create_item(new_item_info);
			}

			GenericRenderObject& render_obj = m_generic_render_pool.objects[canvas_component.render_handle];

			const int32_t render_type = std::clamp(canvas_component.type, 0, Vadon::Utilities::to_integral(RenderObjectType::DIAMOND));

			prepare_canvas_item(render_obj.canvas_item, Vadon::Utilities::to_enum<RenderObjectType>(render_type), canvas_component.color);

			canvas_system.set_item_z_order(render_obj.canvas_item, canvas_component.z_order);
		}

		void remove_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
			if (canvas_component == nullptr)
			{
				// TODO: error!
				return;
			}

			if (canvas_component->render_handle < 0)
			{
				return;
			}

			GenericRenderObject& generic_obj = m_generic_render_pool.objects[canvas_component->render_handle];
			if (generic_obj.canvas_item.is_valid() == false)
			{
				return;
			}

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.remove_item(generic_obj.canvas_item);

			generic_obj.canvas_item.invalidate();

			m_generic_render_pool.unregister_object(canvas_component->render_handle);
			canvas_component->render_handle = -1;
		}

		void set_player_input(Vadon::ECS::World& ecs_world, const PlayerInput& input)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			Player* player_component = component_manager.get_component<Player>(m_player_entity);

			player_component->input = input;
		}
	};

	Model::Model(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(engine_core))
	{
	}

	Model::~Model() = default;

	bool Model::initialize()
	{
		return m_internal->initialize();
	}
	
	bool Model::init_simulation(Vadon::ECS::World& ecs_world)
	{
		return m_internal->init_simulation(ecs_world);
	}

	void Model::update_simulation(Vadon::ECS::World& ecs_world, float delta_time)
	{
		m_internal->update_simulation(ecs_world, delta_time);
	}

	void Model::render_sync(Vadon::ECS::World& ecs_world)
	{
		m_internal->render_sync(ecs_world);
	}

	void Model::update_view_async(Vadon::ECS::World& ecs_world, Vadon::Utilities::PacketQueue& render_queue)
	{
		m_internal->update_view_async(ecs_world, render_queue);
	}

	void Model::render_async(const Vadon::Utilities::PacketQueue& render_queue)
	{
		m_internal->render_async(render_queue);
	}

	void Model::lerp_view_state(float lerp_weight)
	{
		m_internal->lerp_view_state(lerp_weight);
	}

	void Model::update_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->update_canvas_item(ecs_world, entity);
	}

	void Model::remove_canvas_item(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->remove_canvas_item(ecs_world, entity);
	}

	void Model::set_player_input(Vadon::ECS::World& ecs_world, const PlayerInput& input)
	{
		m_internal->set_player_input(ecs_world, input);
	}

	Vadon::Render::Canvas::LayerHandle Model::get_canvas_layer() const
	{
		return m_internal->m_canvas_layer;
	}

	void Model::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);

		// TODO: register types as needed!
	}
}