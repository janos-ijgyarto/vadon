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

	void TextureResource::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<TextureResource, Vadon::Scene::Resource>();

		TypeRegistry::add_property<TextureResource>("texture_file", Vadon::Utilities::MemberVariableBind<&TextureResource::texture_file>().bind_member_getter().bind_member_setter());
	}

	void ShaderResource::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<ShaderResource, Vadon::Scene::Resource>();

		TypeRegistry::add_property<ShaderResource>("shader_file", Vadon::Utilities::MemberVariableBind<&ShaderResource::shader_file>().bind_member_getter().bind_member_setter());
	}
}