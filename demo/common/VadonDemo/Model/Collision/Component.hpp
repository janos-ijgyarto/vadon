#ifndef VADONDEMO_MODEL_COLLISION_COMPONENT_HPP
#define VADONDEMO_MODEL_COLLISION_COMPONENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace Vadon::ECS
{
	class World;
}
namespace VadonDemo::Core
{
	class Core;
}
namespace VadonDemo::Model
{
	enum class CollisionLayer : uint8_t
	{
		PLAYER,
		ENEMIES,
		PROJECTILES
	};

	using CollisionCallback = void(*)(VadonDemo::Core::Core&, Vadon::ECS::World&, Vadon::ECS::EntityHandle, Vadon::ECS::EntityHandle);

	struct Collision
	{
		VADON_DECLARE_MEMBER_UUID(radius, "9a9e38a6-4005-493e-a4ec-f04c2ad8ee17");

		float radius = 1.0f;

		uint8_t layers = 0;
		uint8_t mask = 0;
		CollisionCallback callback = nullptr;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Model::Collision, "b9907bb9-aeb0-4b96-93f3-43764e8415b9");
#endif