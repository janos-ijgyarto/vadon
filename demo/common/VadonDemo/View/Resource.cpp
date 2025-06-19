#include <VadonDemo/View/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewResource::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;

		ResourceRegistry::register_resource_type<ViewResource, Vadon::Scene::Resource>();
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

		TypeRegistry::add_property<Sprite>("texture", Vadon::Utilities::MemberVariableBind<&Sprite::texture>().bind_member_getter().bind_member_setter());
	}
}