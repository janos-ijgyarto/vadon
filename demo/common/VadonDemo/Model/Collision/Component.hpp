#ifndef VADONDEMO_MODEL_COLLISION_COMPONENT_HPP
#define VADONDEMO_MODEL_COLLISION_COMPONENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Model
{
	enum class CollisionLayer : uint8_t
	{
		ENEMIES,
		PROJECTILES
	};

	using CollisionCallback = void(*)(Vadon::ECS::World&, Vadon::ECS::EntityHandle, Vadon::ECS::EntityHandle);

	struct Collision
	{
		float radius = 1.0f;

		uint8_t layers = 0;
		CollisionCallback callback = nullptr;

		static void register_component();
	};
}
#endif