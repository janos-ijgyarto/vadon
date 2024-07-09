#ifndef VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#define VADONEDITOR_MODEL_SCENE_COMPONENT_HPP
#include <Vadon/Utilities/Data/Variant.hpp>
#include <map>
namespace VadonEditor::Model
{
	struct Component
	{
		std::string name;
		std::map<std::string, Vadon::Utilities::Variant> properties;

		bool operator<(const Component& other) const
		{
			return name < other.name;
		}
	};
}
#endif