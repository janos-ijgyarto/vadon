#ifndef VADONDEMO_MODEL_ENEMY_RESOURCE_HPP
#define VADONDEMO_MODEL_ENEMY_RESOURCE_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>
#include <Vadon/Utilities/Enum/EnumClass.hpp>
namespace VadonDemo::Model
{
	struct EnemyDefinition : public Vadon::Scene::Resource
	{
		int32_t score_reward = 0;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyDefinition, EnemyDefinitionID, EnemyDefinitionHandle);

	enum class EnemyMovementType
	{
		DIRECT,
		LOOKAHEAD,
		WEAVING
	};

	struct EnemyMovementDefinition : public Vadon::Scene::Resource
	{
		int32_t movement_type = Vadon::Utilities::to_integral(EnemyMovementType::DIRECT);

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_REFERENCES(EnemyMovementDefinition, EnemyMovementDefID, EnemyMovementDefHandle);

	struct EnemyWeaponAttackDefinition : public Vadon::Scene::Resource
	{
		// TODO: any logic settings?

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