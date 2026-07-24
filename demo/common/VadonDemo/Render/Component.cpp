#include <VadonDemo/Render/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Render
{
	void CanvasComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<CanvasComponent>();

		TypeRegistry::add_property<CanvasComponent>(VADON_GET_MEMBER_UUID(CanvasComponent, visible), Vadon::Utilities::MemberVariableBind<&CanvasComponent::visible>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasComponent>(VADON_GET_MEMBER_UUID(CanvasComponent, z_order), Vadon::Utilities::MemberVariableBind<&CanvasComponent::z_order>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasComponent>(VADON_GET_MEMBER_UUID(CanvasComponent, layer_def), Vadon::Utilities::MemberVariableBind<&CanvasComponent::layer_def>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata canvas_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(CanvasComponent));
		canvas_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::CanvasComponent");
		canvas_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Render");

		{
			Vadon::Utilities::TypePropertyMetadata visible_property(canvas_component_metadata, VADON_GET_MEMBER_UUID(CanvasComponent, visible));
			visible_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Visible");
		}

		{
			Vadon::Utilities::TypePropertyMetadata z_order_property(canvas_component_metadata, VADON_GET_MEMBER_UUID(CanvasComponent, z_order));
			z_order_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Z Order");
		}

		{
			Vadon::Utilities::TypePropertyMetadata layer_def_property(canvas_component_metadata, VADON_GET_MEMBER_UUID(CanvasComponent, layer_def));
			layer_def_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Layer Definition");
		}
	}

	void SpriteTilingComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<SpriteTilingComponent>();

		TypeRegistry::add_property<SpriteTilingComponent>(VADON_GET_MEMBER_UUID(SpriteTilingComponent, texture), Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::texture>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>(VADON_GET_MEMBER_UUID(SpriteTilingComponent, tile_size), Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::tile_size>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>(VADON_GET_MEMBER_UUID(SpriteTilingComponent, repeat), Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::repeat>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>(VADON_GET_MEMBER_UUID(SpriteTilingComponent, rotate), Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::rotate>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata sprite_tiling_metadata(metadata_registry, VADON_GET_TYPE_UUID(SpriteTilingComponent));
		sprite_tiling_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::SpriteTilingComponent");
		sprite_tiling_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Render");

		{
			Vadon::Utilities::TypePropertyMetadata texture_property(sprite_tiling_metadata, VADON_GET_MEMBER_UUID(SpriteTilingComponent, texture));
			texture_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Texture");
		}

		{
			Vadon::Utilities::TypePropertyMetadata tile_size_property(sprite_tiling_metadata, VADON_GET_MEMBER_UUID(SpriteTilingComponent, tile_size));
			tile_size_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Tile Size");
		}

		{
			Vadon::Utilities::TypePropertyMetadata repeat_property(sprite_tiling_metadata, VADON_GET_MEMBER_UUID(SpriteTilingComponent, repeat));
			repeat_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Repeat");
		}

		{
			Vadon::Utilities::TypePropertyMetadata rotate_property(sprite_tiling_metadata, VADON_GET_MEMBER_UUID(SpriteTilingComponent, rotate));
			rotate_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Rotate");
		}
	}

	void FullscreenEffectComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<FullscreenEffectComponent>();

		TypeRegistry::add_property<FullscreenEffectComponent>(VADON_GET_MEMBER_UUID(FullscreenEffectComponent, shader), Vadon::Utilities::MemberVariableBind<&FullscreenEffectComponent::shader>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<FullscreenEffectComponent>(VADON_GET_MEMBER_UUID(FullscreenEffectComponent, view_agnostic), Vadon::Utilities::MemberVariableBind<&FullscreenEffectComponent::view_agnostic>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata fullscreen_effect_metadata(metadata_registry, VADON_GET_TYPE_UUID(FullscreenEffectComponent));
		fullscreen_effect_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Render::FullscreenEffectComponent");
		fullscreen_effect_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Render");

		{
			Vadon::Utilities::TypePropertyMetadata shader_property(fullscreen_effect_metadata, VADON_GET_MEMBER_UUID(FullscreenEffectComponent, shader));
			shader_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Shader");
		}

		{
			Vadon::Utilities::TypePropertyMetadata view_agnostic_property(fullscreen_effect_metadata, VADON_GET_MEMBER_UUID(FullscreenEffectComponent, view_agnostic));
			view_agnostic_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "View Agnostic");
		}
	}
}