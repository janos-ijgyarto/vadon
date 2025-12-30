#ifndef VADONDEMO_MODEL_ENEMY_RESOURCE_HPP
#define VADONDEMO_MODEL_ENEMY_RESOURCE_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
#include <Vadon/Math/Vector.hpp>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Model
{
	struct EnemyDefinition : public Vadon::Scene::Resource
	{
		VADON_DECLARE_MEMBER_UUID(score_reward, "7d1c4df9-bc1b-4d0a-8030-636f5d8e718b");
		int32_t score_reward = 0;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyDefinition, EnemyDefinitionID, EnemyDefinitionHandle);

	struct EnemyMovementDefinition : public Vadon::Scene::Resource
	{
		static void register_resource();

		virtual Vadon::Math::Vector2 get_movement_direction(Vadon::ECS::World& /*ecs_world*/, Vadon::ECS::EntityHandle /*enemy*/, Vadon::ECS::EntityHandle /*target*/, float /*delta_time*/) const { return Vadon::Math::Vector2_One; }
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementDefinition, EnemyMovementDefID, EnemyMovementDefHandle);

	struct EnemyMovementLookahead : public EnemyMovementDefinition
	{
		VADON_DECLARE_MEMBER_UUID(lookahead_factor, "ef5af30c-2726-43b8-a427-0d4e8bf98ef1");
		float lookahead_factor = 1.0f;

		static void register_resource();

		Vadon::Math::Vector2 get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const override;
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementLookahead, EnemyMovementLookaheadID, EnemyMovementLookaheadHandle);

	struct EnemyMovementWeaving : public EnemyMovementDefinition
	{
		VADON_DECLARE_MEMBER_UUID(cycle_duration, "3476be00-2d62-4977-92d4-43b0e1d34427");
		VADON_DECLARE_MEMBER_UUID(weave_factor, "b692eb8d-c744-40be-be93-a6bd4146e5a9");

		float cycle_duration = 0.0f;
		float weave_factor = 0.5f;

		static void register_resource();

		Vadon::Math::Vector2 get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const override;
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementWeaving, EnemyMovementWeavingID, EnemyMovementWeavingHandle);

	struct EnemyWeaponAttackDefinition : public Vadon::Scene::Resource
	{
		VADON_DECLARE_MEMBER_UUID(projectile_count, "9e45319e-e79e-48d6-aeda-8f519c0d593a");
		VADON_DECLARE_MEMBER_UUID(reload_time, "e7748d0c-9c9d-44b4-85a2-fbe239868eb5");

		int projectile_count = 1; // i.e how many are fired each volley
		float reload_time = 1.0f; // i.e how long between volleys

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyWeaponAttackDefinition, EnemyWeaponAttackDefID, EnemyWeaponAttackDefHandle);

	struct EnemyContactDamageDefinition : public Vadon::Scene::Resource
	{
		VADON_DECLARE_MEMBER_UUID(damage, "4153a320-be3b-4996-b2f3-f9b8ad099bcb");

		int damage = 1;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyContactDamageDefinition, EnemyContactDamageDefID, EnemyContactDamageDefHandle);
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyDefinition, "1aac11cc-2779-4b11-8aa7-1d3b3798c254");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyMovementDefinition, "d664bdd4-8728-4ceb-b1ba-8eaca2392dee");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyMovementLookahead, "06957fcb-20c1-4b14-8cff-cbc89f31f71a");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyMovementWeaving, "faef7f32-0e28-46c6-a197-ae4f60d838e7");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyWeaponAttackDefinition, "36bd76a8-bcec-4cde-b931-9fc6c222a66c");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyContactDamageDefinition, "48b15f5a-cf03-4c3a-b6df-33f40d23503c");
#endif