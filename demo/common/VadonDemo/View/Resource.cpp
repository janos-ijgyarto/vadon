#include <VadonDemo/View/Resource.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void RenderResource::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;

		ResourceRegistry::register_resource_type<RenderResource, Vadon::Model::Resource>();

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(RenderResource))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::RenderResource");
	}

	void Shape::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<Shape, RenderResource>();

		TypeRegistry::add_property<Shape>(VADON_GET_MEMBER_UUID(Shape, type), Vadon::Utilities::MemberVariableBind<&Shape::type>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<Shape>(VADON_GET_MEMBER_UUID(Shape, radius), Vadon::Utilities::MemberVariableBind<&Shape::radius>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<Shape>(VADON_GET_MEMBER_UUID(Shape, color), Vadon::Utilities::MemberVariableBind<&Shape::color>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Shape))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::Shape")
			.add_property(VADON_GET_MEMBER_UUID(Shape, type))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Type")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Shape, radius))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Radius")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(Shape, color))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Color")
				.commit_property();
	}

	void Sprite::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<Sprite, RenderResource>();

		TypeRegistry::add_property<Sprite>(VADON_GET_MEMBER_UUID(Sprite, texture), Vadon::Utilities::MemberVariableBind<&Sprite::texture>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(Sprite))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::Sprite")
			.add_property(VADON_GET_MEMBER_UUID(Sprite, texture))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Texture")
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(VadonDemo::Render::TextureResource))
				.commit_property();
	}
}