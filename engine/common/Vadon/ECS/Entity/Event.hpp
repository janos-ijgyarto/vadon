#ifndef VADON_ECS_ENTITY_EVENT_HPP
#define VADON_ECS_ENTITY_EVENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <functional>
namespace Vadon::ECS
{
	enum class EntityEventType
	{
		ADDED,
		REMOVED
		// TODO: any other types?
	};

	struct EntityEvent
	{
		EntityHandle entity;
		EntityEventType event_type;
	};

	using EntityEventCallback = std::function<void(const EntityEvent&)>;
}
#endif