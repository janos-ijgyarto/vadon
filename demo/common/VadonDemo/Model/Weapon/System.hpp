#ifndef VADONDEMO_MODEL_WEAPON_SYSTEM_HPP
#define VADONDEMO_MODEL_WEAPON_SYSTEM_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
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
	struct WeaponComponent;
	struct WeaponDefinition;

	class WeaponSystem
	{
	private:
		WeaponSystem(Core::Core& core);

		static void register_types();

		void update(Vadon::ECS::World& ecs_world, float delta_time);

		void update_weapons(Vadon::ECS::World& ecs_world, float delta_time);
		void update_projectiles(Vadon::ECS::World& ecs_world, float delta_time);

		void create_projectile(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle weapon_entity, const WeaponDefinition* weapon_definition);

		bool validate_weapon(const Player& player, const WeaponComponent& weapon_component);

		static void projectile_collision_callback(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle player, Vadon::ECS::EntityHandle collider);

		Core::Core& m_core;

		friend class Model;
	};
}
#endif