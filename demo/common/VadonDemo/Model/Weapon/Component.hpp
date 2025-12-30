#ifndef VADONDEMO_MODEL_WEAPON_COMPONENT_HPP
#define VADONDEMO_MODEL_WEAPON_COMPONENT_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Math/Vector.hpp>
namespace VadonDemo::Model
{
	struct WeaponComponent
	{
		VADON_DECLARE_MEMBER_UUID(definition, "d6318ecc-b5d5-4952-ad44-2e266b7eac7a");
		WeaponDefID definition;

		float firing_timer = 0.0f;

		static void register_component();
	};

	struct WeaponVolleyComponent
	{
		VADON_DECLARE_MEMBER_UUID(fire_count, "fbbcdbe6-921f-4a1b-a465-dd25d5332e3b");
		uint32_t fire_count = 0;

		static void register_component();
	};

	struct ProjectileComponent
	{
		VADON_DECLARE_MEMBER_UUID(range, "0f9149d0-ba4f-4845-950b-de47c3402434");
		VADON_DECLARE_MEMBER_UUID(damage, "be270915-1496-4893-9549-f7e074ec92e5");
		VADON_DECLARE_MEMBER_UUID(knockback, "37fc76aa-a390-4bbe-8a1f-c08193fb2d8d");

		float range = 0.0f;
		float damage = 0.0f;
		float knockback = 0.0f;

		float remaining_lifetime = 0.0f;
		bool enemy = false; // TODO: hack to get it working, need to improve this!

		static void register_component();
	};

	struct ProjectileHomingComponent
	{
		VADON_DECLARE_MEMBER_UUID(turn_speed, "1f8a795e-0c48-4e78-a1c2-85116849ddcc");
		VADON_DECLARE_MEMBER_UUID(aimed_on_launch, "fa193fc0-4b33-4dff-ba52-f47e1acd1e8c");

		static constexpr float c_min_turn_speed = 0.1f;
		float turn_speed = c_min_turn_speed;
		// TODO: implement flags?
		bool aimed_on_launch = false;

		Vadon::ECS::EntityHandle target_entity;

		static void register_component();
	};

	struct ProjectileAOEComponent
	{
		VADON_DECLARE_MEMBER_UUID(radius, "bcf7bc1c-ccc4-4f53-ab47-e724a200d43d");

		float radius = 0.0f;

		static void register_component();
	};

	struct ProjectileExplosionTag {};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::WeaponComponent, "38a0495a-214c-4315-840e-1d43ed6d27e4");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::WeaponVolleyComponent, "5345f9e9-8ca9-4041-9a09-8ae406c4ed40");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::ProjectileComponent, "6be89adf-23b1-48a6-8e1b-32bef68c8a24");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::ProjectileHomingComponent, "2b984d44-ac7f-4b80-affa-907b128bcbc9");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::ProjectileAOEComponent, "abd8e3b7-8b76-4bc4-8fa0-6513526d233e");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::ProjectileExplosionTag, "81369760-8b8a-4bb4-a226-6342b5c10616");

#endif