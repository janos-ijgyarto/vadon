#ifndef VADONDEMO_MODEL_COMPONENT_HPP
#define VADONDEMO_MODEL_COMPONENT_HPP
#include <VadonDemo/Model/Resource.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>

namespace VadonDemo::Model
{
	struct Transform2D
	{
		Vadon::Utilities::Vector2 position = Vadon::Utilities::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		bool teleported = false;

		static void register_component();
	};

	struct Velocity2D
	{
		float top_speed = 0.0f;
		float acceleration = 0.0f;
		Vadon::Utilities::Vector2 velocity = Vadon::Utilities::Vector2_Zero;

		static void register_component();
	};

	struct Collision
	{
		float radius = 1.0f;
		size_t collision_data_offset = 0;

		static void register_component();
	};

	struct Health
	{
		float max_health = 0.0f;
		float current_health = 0.0f;

		static void register_component();
	};

	struct PlayerInput
	{
		Vadon::Utilities::Vector2 move_dir = Vadon::Utilities::Vector2_Zero;
	};

	struct Player
	{
		float damage_delay = 0.0f;
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		std::vector<WeaponDefID> starting_weapons;

		int score = 0;
		PlayerInput input;
		Vadon::Utilities::Vector2 last_move_dir = { 1, 0 };

		float damage_timer = 0.0f;

		static void register_component();
	};

	struct Weapon
	{
		WeaponDefID definition;

		float firing_timer = 0.0f;
		Vadon::Utilities::Vector2 aim_direction = { 1, 0 };

		static void register_component();
	};

	struct Projectile
	{
		float range = 0.0f;
		float damage = 0.0f;
		float remaining_lifetime = 0.0f;

		static void register_component();
	};

	struct Enemy
	{
		int32_t score_reward = 0;
		float damage = 0.0f;

		static void register_component();
	};

	struct Map
	{
		std::string display_name;
		Vadon::Utilities::Vector2 dimensions = Vadon::Utilities::Vector2_Zero;
		// TODO: limit on spawned enemies (could do with some kind of "value" system where it limits number based on how much they add up to?)

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