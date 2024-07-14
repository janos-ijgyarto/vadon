#ifndef VADON_ECS_ENTITY_ENTITY_HPP
#define VADON_ECS_ENTITY_ENTITY_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <span>
namespace Vadon::ECS
{
	VADON_DECLARE_TYPED_POOL_HANDLE(Entity, EntityHandle);
	using EntityList = std::vector<EntityHandle>;
	using EntitySpan = std::span<const EntityHandle>;
}
#endif