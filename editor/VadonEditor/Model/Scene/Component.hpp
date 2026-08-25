#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <VadonEditor/Model/Property.hpp>
#include <Vadon/ECS/Component/Component.hpp>
namespace VadonEditor::Model
{
	struct ComponentInfo
	{
		Vadon::ECS::ComponentID type_id;
		std::string name;
		// TODO: other metadata?
	};

	struct Component
	{
		std::string name;
		std::vector<Property> properties;
	};
}
#endif