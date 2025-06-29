#ifndef VADONDEMO_MODEL_MODEL_HPP
#define VADONDEMO_MODEL_MODEL_HPP
#include <VadonDemo/VadonDemoCommon.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <random>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
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
	struct Weapon;

	class Model
	{
	public:
		static void register_types();

		VADONDEMO_API bool init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);
		VADONDEMO_API void update(Vadon::ECS::World& ecs_world, float delta_time);
		VADONDEMO_API bool is_in_end_state(Vadon::ECS::World& ecs_world) const;
		VADONDEMO_API void end_simulation(Vadon::ECS::World& ecs_world);
	private:
		Model(Core::Core& core);
		bool initialize();
		void global_config_updated();

		bool internal_init_simulation(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);
		bool load_level(Vadon::ECS::World& ecs_world, Vadon::Scene::SceneID level_scene_id);

		bool validate_sim_state(Vadon::ECS::World& ecs_world);
		bool validate_weapon(const Player& player, const Weapon& weapon_component);
		bool validate_spawner(const Spawner& spawner);
		void deferred_remove_entity(Vadon::ECS::EntityHandle entity_handle);

		void update_player(Vadon::ECS::World& ecs_world, float delta_time);
		void update_enemies(Vadon::ECS::World& ecs_world, float delta_time);
		void update_dynamic(Vadon::ECS::World& ecs_world, float delta_time);
		void update_collisions(Vadon::ECS::World& ecs_world, float delta_time);
		void update_health(Vadon::ECS::World& ecs_world, float delta_time);
		void update_spawners(Vadon::ECS::World& ecs_world, float delta_time);
		void update_weapons(Vadon::ECS::World& ecs_world, float delta_time);
		void update_projectiles(Vadon::ECS::World& ecs_world, float delta_time);

		void clear_removed_entities(Vadon::ECS::World& ecs_world);

		Core::Core& m_core;

		std::mt19937 m_random_engine;
		std::uniform_real_distribution<float> m_enemy_dist;

		std::vector<Vadon::ECS::EntityHandle> m_entity_remove_list;

		Vadon::ECS::EntityHandle m_level_root;

		friend Core::Core;
	};
}
#endif