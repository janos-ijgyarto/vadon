#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewComponent>();

		TypeRegistry::add_property<ViewComponent>("resource", Vadon::Utilities::MemberVariableBind<&ViewComponent::resource>().bind_member_getter().bind_member_setter());
	}

	void AnimationComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<AnimationComponent>();

		TypeRegistry::add_property<AnimationComponent>("time_scale", Vadon::Utilities::MemberVariableBind<&AnimationComponent::time_scale>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationComponent>("looping", Vadon::Utilities::MemberVariableBind<&AnimationComponent::looping>().bind_member_getter().bind_member_setter());
	}

	void DamageComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<DamageComponent>();

		TypeRegistry::add_property<DamageComponent>("animation", Vadon::Utilities::MemberVariableBind<&DamageComponent::animation>().bind_member_getter().bind_member_setter());
	}

	void VFXComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<VFXComponent>();

		TypeRegistry::add_property<VFXComponent>("vfx_prefab", Vadon::Utilities::MemberVariableBind<&VFXComponent::vfx_prefab>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>("animation", Vadon::Utilities::MemberVariableBind<&VFXComponent::animation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>("lifetime", Vadon::Utilities::MemberVariableBind<&VFXComponent::lifetime>().bind_member_getter().bind_member_setter());
	}

	void VFXTimerComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<DamageComponent>();

		TypeRegistry::add_property<DamageComponent>("animation", Vadon::Utilities::MemberVariableBind<&DamageComponent::animation>().bind_member_getter().bind_member_setter());
	}
}