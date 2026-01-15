#ifndef VADONEDITOR_SCENE_EVENT_HPP
#define VADONEDITOR_SCENE_EVENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace VadonEditor::Scene
{
	enum class EntityEventType
	{
		ADDED,
		REMOVED
	};

	struct EntityEvent
	{
		Vadon::ECS::EntityHandle entity;
		EntityEventType type;
	};

	enum class ComponentEventType
	{
		ADDED,
		EDITED,
		REMOVED
	};

	struct ComponentEvent
	{
		ComponentEventType type;
		Vadon::ECS::EntityHandle owner;
		Vadon::Utilities::TypeID component_type;
		// TODO: property name/ID in case it was an edit?
	};
}
#endif