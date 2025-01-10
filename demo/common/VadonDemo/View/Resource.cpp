#include <VadonDemo/View/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewResource::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<ViewResource, Vadon::Scene::Resource>();

		TypeRegistry::add_property<ViewResource>("z_order", Vadon::Utilities::MemberVariableBind<&ViewResource::z_order>().bind_member_getter().bind_member_setter());
	}

	void Shape::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<Shape, ViewResource>();

		TypeRegistry::add_property<Shape>("type", Vadon::Utilities::MemberVariableBind<&Shape::type>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<Shape>("color", Vadon::Utilities::MemberVariableBind<&Shape::color>().bind_member_getter().bind_member_setter());
	}

	void Sprite::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<Sprite, ViewResource>();

		TypeRegistry::add_property<Sprite>("texture_path", Vadon::Utilities::MemberVariableBind<&Sprite::texture_path>().bind_member_getter().bind_member_setter());
	}
}