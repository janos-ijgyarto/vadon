#ifndef VADONEDITOR_MODEL_SCENE_EVENT_HPP
#define VADONEDITOR_MODEL_SCENE_EVENT_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace VadonEditor::Model
{
	enum class EntityEventType
	{
		ADDED, // Dispatched immediately after Entity is fully initialized
		REMOVED // Dispatched immediately before Entity is removed
	};

	struct EntityEvent
	{
		EntityEventType type;
		Vadon::ECS::EntityHandle entity;
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