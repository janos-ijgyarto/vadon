#ifndef VADONDEMO_MODEL_COLLISION_SYSTEM_HPP
#define VADONDEMO_MODEL_COLLISION_SYSTEM_HPP
#include <VadonDemo/Model/Collision/Component.hpp>
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
	class CollisionSystem
	{
	private:
		CollisionSystem(Core::Core& core);

		static void register_types(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);

		void update(Vadon::ECS::World& ecs_world, float delta_time);

		Core::Core& m_core;

		friend class Model;
	};
}
#endif