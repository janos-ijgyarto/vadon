#ifndef VADONDEMO_MODEL_ENEMY_COMPONENT_HPP
#define VADONDEMO_MODEL_ENEMY_COMPONENT_HPP
#include <VadonDemo/Model/Enemy/Resource.hpp>
namespace VadonDemo::Model
{
	struct EnemyBase
	{
		EnemyDefinitionID definition;
		// TODO: anything else?

		static void register_component();
	};

	struct EnemyMovement
	{
		EnemyMovementDefID definition;

		static void register_component();
	};

	// Enemy attacks with a weapon
	struct EnemyWeapon
	{
		EnemyWeaponAttackDefID definition;

		static void register_component();
	};

	// Enemy just deals damage on contact
	struct EnemyContactDamage
	{
		EnemyContactDamageDefID definition;

		static void register_component();
	};

	struct Spawner
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		Vadon::Scene::SceneID enemy_prefab;

		float activation_delay = 60.0f;
		float min_spawn_delay = 0.0f;
		int32_t start_spawn_count = 1;
		// TODO: spawn rate!

		float level_multiplier = 1.0f;
		float level_up_delay = 30.0f;
		int32_t max_level = 1;

		int32_t current_level = 0;
		int32_t current_spawn_count = 1;
		float spawn_timer = 0.0f;
		float level_up_timer = 0.0f;

		static void register_component();
	};
}
#endif