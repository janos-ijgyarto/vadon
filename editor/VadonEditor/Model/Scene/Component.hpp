#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <Vadon/Utilities/TypeInfo/Registry/Property.hpp>
#include <map>
namespace VadonEditor::Model
{
	struct Component
	{
		std::string name;
		Vadon::Utilities::PropertyList properties;
	};
}
#endif