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
		float lookahead_factor = 1.0f;

		static void register_resource();

		Vadon::Math::Vector2 get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const override;
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementLookahead, EnemyMovementLookaheadID, EnemyMovementLookaheadHandle);

	struct EnemyMovementWeaving : public EnemyMovementDefinition
	{
		float cycle_duration = 0.0f;
		float weave_factor = 0.5f;

		static void register_resource();

		Vadon::Math::Vector2 get_movement_direction(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle enemy, Vadon::ECS::EntityHandle target, float delta_time) const override;
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementWeaving, EnemyMovementWeavingID, EnemyMovementWeavingHandle);

	struct EnemyWeaponAttackDefinition : public Vadon::Scene::Resource
	{
		int projectile_count = 1; // i.e how many are fired each volley
		float reload_time = 1.0f; // i.e how long between volleys

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyWeaponAttackDefinition, EnemyWeaponAttackDefID, EnemyWeaponAttackDefHandle);

	struct EnemyContactDamageDefinition : public Vadon::Scene::Resource
	{
		int damage = 1;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyContactDamageDefinition, EnemyContactDamageDefID, EnemyContactDamageDefHandle);
}
#endif