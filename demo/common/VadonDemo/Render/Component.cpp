#include <VadonDemo/Render/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Render
{
	void CanvasComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<CanvasComponent>();

		TypeRegistry::add_property<CanvasComponent>("visible", Vadon::Utilities::MemberVariableBind<&CanvasComponent::visible>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasComponent>("z_order", Vadon::Utilities::MemberVariableBind<&CanvasComponent::z_order>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<CanvasComponent>("layer_def", Vadon::Utilities::MemberVariableBind<&CanvasComponent::layer_def>().bind_member_getter().bind_member_setter());
	}

	void SpriteTilingComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<SpriteTilingComponent>();

		TypeRegistry::add_property<SpriteTilingComponent>("texture", Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::texture>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>("tile_size", Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::tile_size>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>("repeat", Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::repeat>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<SpriteTilingComponent>("rotate", Vadon::Utilities::MemberVariableBind<&SpriteTilingComponent::rotate>().bind_member_getter().bind_member_setter());
	}

	void FullscreenEffectComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<FullscreenEffectComponent>();

		TypeRegistry::add_property<FullscreenEffectComponent>("shader", Vadon::Utilities::MemberVariableBind<&FullscreenEffectComponent::shader>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<FullscreenEffectComponent>("view_agnostic", Vadon::Utilities::MemberVariableBind<&FullscreenEffectComponent::view_agnostic>().bind_member_getter().bind_member_setter());
	}
}