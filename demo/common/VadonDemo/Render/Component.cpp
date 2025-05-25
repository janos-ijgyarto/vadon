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

	void FullscreenEffectComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<FullscreenEffectComponent>();

		TypeRegistry::add_property<FullscreenEffectComponent>("effect", Vadon::Utilities::MemberVariableBind<&FullscreenEffectComponent::effect>().bind_member_getter().bind_member_setter());
	}
}