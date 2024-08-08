#ifndef VADON_ECS_COMPONENT_COMPONENT_HPP
#define VADON_ECS_COMPONENT_COMPONENT_HPP
#include <Vadon/Utilities/TypeInfo/TypeInfo.hpp>
namespace Vadon::ECS
{
	using ComponentID = Vadon::Utilities::TypeID;
	using ComponentIDList = std::vector<ComponentID>;
}
#endif