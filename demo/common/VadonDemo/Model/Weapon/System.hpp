#ifndef VADONDEMO_MODEL_WEAPON_SYSTEM_HPP
#define VADONDEMO_MODEL_WEAPON_SYSTEM_HPP
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

	class WeaponSystem
	{
	private:
		WeaponSystem(Core::Core& core);

		static void register_types();

		void update(Vadon::ECS::World& ecs_world, float delta_time);

		void update_weapons(Vadon::ECS::World& ecs_world, float delta_time);
		void update_projectiles(Vadon::ECS::World& ecs_world, float delta_time);

		bool validate_weapon(const Player& player, const WeaponComponent& weapon_component);

		Core::Core& m_core;

		friend class Model;
	};
}
#endif