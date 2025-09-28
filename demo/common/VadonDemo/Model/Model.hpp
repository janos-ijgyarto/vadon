#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <VadonDemo/Model/Collision/System.hpp>
#include <VadonDemo/Model/Weapon/System.hpp>

#include <Vadon/Math/Vector.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <random>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class Core;
}
namespace VadonDemo::Model
{
	struct Player;
	struct PlayerInput;
	struct Spawner;

	class Model
	{
	public:
		static void register_types();

		VADONDEMO_API bool init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);
		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, float delta_time);
		VADONDEMO_API bool is_in_end_state(Vadon::ECS::World& ecs_world) const;
		VADONDEMO_API void end_simulation(Vadon::ECS::World& ecs_world);

		// FIXME: implement utility function to handle this (i.e "find first entity with tag T")
		VADONDEMO_API static Vadon::ECS::EntityHandle get_root_entity(Vadon::ECS::World& ecs_world);

		float get_elapsed_time() const { return m_elapsed_time; }

		VADONDEMO_API void init_entity_collision(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
	private:
		Model(Core::Core& core);
		bool initialize();
		void global_config_updated();

		bool internal_init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);
		bool load_level(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);

		bool validate_sim_state(Vadon::ECS::World& ecs_world);
		bool validate_spawner(const Spawner& spawner);

		void update_player(Vadon::ECS::World& ecs_world, float delta_time);
		void update_enemies(Vadon::ECS::World& ecs_world, float delta_time);
		void update_dynamic(Vadon::ECS::World& ecs_world, float delta_time);
		void update_health(Vadon::ECS::World& ecs_world, float delta_time);
		void update_spawners(Vadon::ECS::World& ecs_world, float delta_time);

		void spawn_enemy(Vadon::ECS::World& ecs_world, const Spawner& spawner, const Vadon::Math::Vector2& position);

		static void player_collision_callback(VadonDemo::Core::Core& core, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle collider);
		static void enemy_collision_callback(VadonDemo::Core::Core& core, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle collider);

		void clear_removed_entities(Vadon::ECS::World& ecs_world);

		Core::Core& m_core;

		CollisionSystem m_collision_system;
		WeaponSystem m_weapon_system;

		std::mt19937 m_random_engine;
		std::uniform_real_distribution<float> m_enemy_dist;

		float m_elapsed_time;

		friend Core::Core;
	};
}
#endif