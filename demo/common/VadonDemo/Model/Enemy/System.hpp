#ifndef VADONDEMO_MODEL_ENEMY_SYSTEM_HPP
#define VADONDEMO_MODEL_ENEMY_SYSTEM_HPP
#include <Vadon/ECS/Component/Component.hpp>
#include <Vadon/Math/Vector.hpp>
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
	struct Spawner;

	class EnemySystem
	{
	private:
		EnemySystem(Core::Core& core);

		static void register_types();

		bool init_collisions(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		static void enemy_collision_callback(VadonDemo::Core::Core& core, Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle collider);

		bool validate_sim_state() const;
		bool validate_spawner(const Vadon::ECS::TypedComponentHandle<Spawner>& spawner) const;

		void update(Vadon::ECS::World& ecs_world, float delta_time);

		void update_enemies(Vadon::ECS::World& ecs_world, float delta_time);
		void update_spawners(Vadon::ECS::World& ecs_world, float delta_time);

		void spawn_enemy(Vadon::ECS::World& ecs_world, const Vadon::ECS::TypedComponentHandle<Spawner>& spawner, const Vadon::Math::Vector2& position);

		void enemy_player_contact(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle enemy);

		Core::Core& m_core;
		std::uniform_real_distribution<float> m_enemy_dist;

		friend class Model;
	};
}
#endif