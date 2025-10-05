#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void WeaponComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<WeaponComponent>();

		Vadon::Utilities::TypeRegistry::add_property<WeaponComponent>("definition", Vadon::Utilities::MemberVariableBind<&WeaponComponent::definition>().bind_member_getter().bind_member_setter());
	}

	void ProjectileComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>("range", Vadon::Utilities::MemberVariableBind<&ProjectileComponent::range>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>("damage", Vadon::Utilities::MemberVariableBind<&ProjectileComponent::damage>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>("knockback", Vadon::Utilities::MemberVariableBind<&ProjectileComponent::knockback>().bind_member_getter().bind_member_setter());
	}

	void ProjectileHomingComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileHomingComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>("turn_speed", Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::turn_speed>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>("aimed_on_launch", Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::aimed_on_launch>().bind_member_getter().bind_member_setter());
	}

	void ProjectileAOEComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileAOEComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileAOEComponent>("radius", Vadon::Utilities::MemberVariableBind<&ProjectileAOEComponent::radius>().bind_member_getter().bind_member_setter());
	}
}