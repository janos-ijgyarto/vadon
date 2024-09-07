#ifndef VADON_ECS_WORLD_EVENT_HPP
#define VADON_ECS_WORLD_EVENT_HPP
#include <functional>
namespace Vadon::ECS
{
	enum class WorldEventType
	{
		CLEARED
		// TODO: any other types?
	};

	struct WorldEvent
	{
		WorldEventType event_type;
	};

	using WorldEventCallback = std::function<void(const WorldEvent&)>;
}
#endif