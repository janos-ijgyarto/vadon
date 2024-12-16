#include <VadonDemo/Model/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::Model
{
	void CanvasItemDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<CanvasItemDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<CanvasItemDefinition>("type", Vadon::Utilities::MemberVariableBind<&CanvasItemDefinition::type>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasItemDefinition>("color", Vadon::Utilities::MemberVariableBind<&CanvasItemDefinition::color>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasItemDefinition>("z_order", Vadon::Utilities::MemberVariableBind<&CanvasItemDefinition::z_order>().bind_member_getter().bind_member_setter());
	}
}