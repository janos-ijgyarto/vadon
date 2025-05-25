#include <VadonDemo/Render/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Render
{
	void CanvasLayerDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<CanvasLayerDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<CanvasLayerDefinition>("priority", Vadon::Utilities::MemberVariableBind<&CanvasLayerDefinition::priority>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasLayerDefinition>("view_agnostic", Vadon::Utilities::MemberVariableBind<&CanvasLayerDefinition::view_agnostic>().bind_member_getter().bind_member_setter());
	}

	void FullscreenEffect::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<FullscreenEffect, Vadon::Scene::Resource>();

		TypeRegistry::add_property<FullscreenEffect>("shader_path", Vadon::Utilities::MemberVariableBind<&FullscreenEffect::shader_path>().bind_member_getter().bind_member_setter());
	}
}