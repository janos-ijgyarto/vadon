#ifndef VADONDEMO_MODEL_COMPONENT_HPP
#define VADONDEMO_MODEL_COMPONENT_HPP
#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Math/Vector.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::Model
{
	struct LevelRootTag {};
	struct DestroyEntityTag {};

	struct Transform2D
	{
		VADON_DECLARE_MEMBER_UUID(position, "ea9ad9fe-ee5d-4ff7-8fdd-e3dd851cf841");
		VADON_DECLARE_MEMBER_UUID(rotation, "945cc310-09fb-45c8-bec8-0d404742e963");
		VADON_DECLARE_MEMBER_UUID(scale, "aca7e61e-0516-4f18-9c41-8f7ed7682e9c");

		Vadon::Math::Vector2 position = Vadon::Math::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		bool teleported = false;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Velocity2D
	{
		VADON_DECLARE_MEMBER_UUID(top_speed, "da0507c5-595b-4658-864c-893794e0a392");
		VADON_DECLARE_MEMBER_UUID(acceleration, "be1c6564-f3b3-44c4-a052-0772ab12f033");

		// FIXME: Velocity should not define these, should instead delegate to other logic
		// This component should only have runtime values that are used to update the transform
		float top_speed = 0.0f;
		float acceleration = 0.0f;
		Vadon::Math::Vector2 velocity = Vadon::Math::Vector2_Zero;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Health
	{
		VADON_DECLARE_MEMBER_UUID(max_health, "e0ee754b-e2e8-4df4-8e34-81c8d7e54f34");

		float max_health = 0.0f;
		float current_health = 0.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct PlayerInput
	{
		Vadon::Math::Vector2 move_dir = Vadon::Math::Vector2_Zero;
	};

	struct Player
	{
		VADON_DECLARE_MEMBER_UUID(damage_delay, "67d32c3d-390b-4d6f-82f2-33e8c7a8b47d");
		VADON_DECLARE_MEMBER_UUID(starting_weapons, "94e849f1-fa63-4556-8139-b844dd2d292c");

		float damage_delay = 0.0f;
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		std::vector<WeaponDefID> starting_weapons;

		int score = 0;
		PlayerInput input;
		Vadon::Math::Vector2 last_move_dir = { 1, 0 };

		float damage_timer = 0.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct Map
	{
		VADON_DECLARE_MEMBER_UUID(display_name, "19782d6e-e696-4df2-8f79-227f6269438a");
		VADON_DECLARE_MEMBER_UUID(dimensions, "5f14f42a-a0bc-4d64-8ab3-e2b364262890");

		std::string display_name;
		Vadon::Math::Vector2 dimensions = Vadon::Math::Vector2_Zero;
		// TODO: limit on spawned enemies (could do with some kind of "value" system where it limits number based on how much they add up to?)

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::LevelRootTag, "2e698839-a636-45b7-9236-185fd1691045");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::DestroyEntityTag, "74c06abf-6aa8-4181-bf03-3b62dfd2dcd9");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Transform2D, "e0c7e9d3-4788-4f26-8236-449698c34415");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Velocity2D, "ea7623d3-174a-425c-aa50-ca9c81d2610c");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Health, "e4b8b721-3e6e-45b9-98d3-9f251176f39d");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Player, "ac3c9ae1-71dc-41d8-8c29-2f2c15c51462");
VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Map, "7894f2ce-5817-4fc8-a0de-7d6274ce8747");
#endif