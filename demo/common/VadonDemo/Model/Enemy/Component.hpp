#ifndef VADONDEMO_MODEL_ENEMY_COMPONENT_HPP
#define VADONDEMO_MODEL_ENEMY_COMPONENT_HPP
#include <VadonDemo/Model/Enemy/Resource.hpp>
namespace VadonDemo::Model
{
	struct EnemyBase
	{
		VADON_DECLARE_MEMBER_UUID(definition, "5e2929d5-4762-456d-8e75-e2e4a08eec89");
		EnemyDefinitionID definition;
		// TODO: anything else?

		static void register_component();
	};

	struct EnemyMovement
	{
		VADON_DECLARE_MEMBER_UUID(definition, "eb4e2d70-cc9e-429d-bcc9-da8bcc4ffc3d");

		EnemyMovementDefID definition;
		EnemyMovementDefHandle def_handle;
		float data = 0; // FIXME: this is to enable weaving, need a more modular solution!

		static void register_component();
	};

	// Enemy attacks with a weapon
	struct EnemyWeapon
	{
		VADON_DECLARE_MEMBER_UUID(definition, "34aca668-48eb-42bf-afd6-520e908410c3");

		EnemyWeaponAttackDefID definition;
		EnemyWeaponAttackDefHandle def_handle;

		float reload_timer = 0.0f;

		static void register_component();
	};

	// Enemy just deals damage on contact
	struct EnemyContactDamage
	{
		VADON_DECLARE_MEMBER_UUID(definition, "49c8f694-b86c-467a-983f-f35e32a3908b");
		EnemyContactDamageDefID definition;

		static void register_component();
	};

	struct Spawner
	{
		VADON_DECLARE_MEMBER_UUID(enemy_prefab, "cb03b39f-6a1a-4ebb-9bb4-086774aa56e1");
		VADON_DECLARE_MEMBER_UUID(activation_delay, "caa57fc0-f408-40f4-a375-7f68b7b145e9");
		VADON_DECLARE_MEMBER_UUID(min_spawn_delay, "d48706a2-7e32-437a-bc54-79e56b4de27f");
		VADON_DECLARE_MEMBER_UUID(start_spawn_count, "3c891a27-c3e1-4e35-b73e-3b6e548089c2");
		VADON_DECLARE_MEMBER_UUID(level_multiplier, "bef57cf2-cb36-4a3a-9bc1-012fda04f8d6");
		VADON_DECLARE_MEMBER_UUID(level_up_delay, "ded6c586-8290-442d-92b2-df287a6625cf");
		VADON_DECLARE_MEMBER_UUID(max_level, "7ea6af40-feea-4667-b191-c879af151d3c");

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

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyBase, "aacd6bae-b782-4232-ba13-860f0e595f01");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyMovement, "f98886d7-01c9-4c9a-b6d6-4929ee6653f2");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyWeapon, "66a1539c-f664-4952-aa10-54072e10cbc9");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::EnemyContactDamage, "33dd507d-ff86-4062-8af7-7b1a7ac70b4a");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Spawner, "39e46d5d-16a1-4ae1-bbe8-699758890a9b");
#endif