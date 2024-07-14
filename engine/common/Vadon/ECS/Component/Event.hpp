#ifndef VADON_ECS_COMPONENT_EVENT_HPP
#define VADON_ECS_COMPONENT_EVENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <functional>
namespace Vadon::ECS
{
	enum class ComponentEventType
	{
		ADDED,
		REMOVED
		// TODO: add event for modifying properties?
	};

	struct ComponentEvent
	{
		EntityHandle owner;
		ComponentID type_id;
		ComponentEventType event_type;
	};

	using ComponentEventCallback = std::function<void(const ComponentEvent&)>;
}
#endif