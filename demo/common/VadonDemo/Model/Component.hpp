#ifndef VADONDEMO_MODEL_COMPONENT_HPP
#define VADONDEMO_MODEL_COMPONENT_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Math/Vector.hpp>

namespace VadonDemo::Model
{
	struct LevelRootTag {};
	struct DestroyEntityTag {};

	struct Transform2D
	{
		Vadon::Math::Vector2 position = Vadon::Math::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		bool teleported = false;

		static void register_component();
	};

	struct Velocity2D
	{
		// FIXME: Velocity should not define these, should instead delegate to other logic
		// This component should only have runtime values that are used to update the transform
		float top_speed = 0.0f;
		float acceleration = 0.0f;
		Vadon::Math::Vector2 velocity = Vadon::Math::Vector2_Zero;

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
		Vadon::Math::Vector2 move_dir = Vadon::Math::Vector2_Zero;
	};

	struct Player
	{
		float damage_delay = 0.0f;
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		std::vector<WeaponDefID> starting_weapons;

		int score = 0;
		PlayerInput input;
		Vadon::Math::Vector2 last_move_dir = { 1, 0 };

		float damage_timer = 0.0f;

		static void register_component();
	};

	struct Map
	{
		std::string display_name;
		Vadon::Math::Vector2 dimensions = Vadon::Math::Vector2_Zero;
		// TODO: limit on spawned enemies (could do with some kind of "value" system where it limits number based on how much they add up to?)

		static void register_component();
	};
}
#endif