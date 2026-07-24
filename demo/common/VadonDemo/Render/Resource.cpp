#include <VadonDemo/Render/Resource.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Render
{
	void CanvasLayerDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<CanvasLayerDefinition, Vadon::Model::Resource>();

		TypeRegistry::add_property<CanvasLayerDefinition>(VADON_GET_MEMBER_UUID(CanvasLayerDefinition, priority), Vadon::Utilities::MemberVariableBind<&CanvasLayerDefinition::priority>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasLayerDefinition>(VADON_GET_MEMBER_UUID(CanvasLayerDefinition, view_agnostic), Vadon::Utilities::MemberVariableBind<&CanvasLayerDefinition::view_agnostic>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(CanvasLayerDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::CanvasLayerDefinition")
			.add_property(VADON_GET_MEMBER_UUID(CanvasLayerDefinition, priority))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Priority")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(CanvasLayerDefinition, view_agnostic))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "View Agnostic")
				.commit_property();
	}

	void TextureResource::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<TextureResource, Vadon::Model::Resource>();

		TypeRegistry::add_property<TextureResource>(VADON_GET_MEMBER_UUID(TextureResource, texture_file), Vadon::Utilities::MemberVariableBind<&TextureResource::texture_file>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(TextureResource))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::TextureResource")
			.add_property(VADON_GET_MEMBER_UUID(TextureResource, texture_file))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Texture File")
				.commit_property();
	}

	void ShaderResource::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<ShaderResource, Vadon::Model::Resource>();

		TypeRegistry::add_property<ShaderResource>(VADON_GET_MEMBER_UUID(ShaderResource, shader_file), Vadon::Utilities::MemberVariableBind<&ShaderResource::shader_file>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(ShaderResource))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::ShaderResource")
			.add_property(VADON_GET_MEMBER_UUID(ShaderResource, shader_file))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Shader File")
				.commit_property();
	}
}