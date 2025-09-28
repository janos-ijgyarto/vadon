#ifndef VADONDEMO_MODEL_WEAPON_COMPONENT_HPP
#define VADONDEMO_MODEL_WEAPON_COMPONENT_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Math/Vector.hpp>
namespace VadonDemo::Model
{
	struct WeaponComponent
	{
		WeaponDefID definition;

		Vadon::ECS::EntityHandle target_entity;
		Vadon::Math::Vector2 travel_direction;

		float firing_timer = 0.0f;

		static void register_component();
	};

	struct ProjectileComponent
	{
		float range = 0.0f;
		float damage = 0.0f;
		float knockback = 0.0f;

		float remaining_lifetime = 0.0f;

		static void register_component();
	};

	struct ProjectileHomingComponent
	{
		static constexpr float c_min_turn_speed = 0.1f;
		float turn_speed = c_min_turn_speed;
		// TODO: implement flags?
		bool aimed_on_launch = false;

		Vadon::ECS::EntityHandle target_entity;

		static void register_component();
	};

	struct ProjectileAOEComponent
	{
		float radius = 0.0f;

		static void register_component();
	};

	struct ProjectileExplosionTag {};
}
#endif