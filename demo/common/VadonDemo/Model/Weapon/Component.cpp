#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void WeaponComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<WeaponComponent>();

		Vadon::Utilities::TypeRegistry::add_property<WeaponComponent>(VADON_GET_MEMBER_UUID(WeaponComponent, definition), Vadon::Utilities::MemberVariableBind<&WeaponComponent::definition>().bind_member_getter().bind_member_setter());
	}

	void WeaponVolleyComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<WeaponVolleyComponent>();

		Vadon::Utilities::TypeRegistry::add_property<WeaponVolleyComponent>(VADON_GET_MEMBER_UUID(WeaponVolleyComponent, fire_count), Vadon::Utilities::MemberVariableBind<&WeaponVolleyComponent::fire_count>().bind_member_getter().bind_member_setter());
	}

	void ProjectileComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, range), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::range>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, damage), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::damage>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, knockback), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::knockback>().bind_member_getter().bind_member_setter());
	}

	void ProjectileHomingComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileHomingComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>(VADON_GET_MEMBER_UUID(ProjectileHomingComponent, turn_speed), Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::turn_speed>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>(VADON_GET_MEMBER_UUID(ProjectileHomingComponent, aimed_on_launch), Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::aimed_on_launch>().bind_member_getter().bind_member_setter());
	}

	void ProjectileAOEComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileAOEComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileAOEComponent>(VADON_GET_MEMBER_UUID(ProjectileAOEComponent, radius), Vadon::Utilities::MemberVariableBind<&ProjectileAOEComponent::radius>().bind_member_getter().bind_member_setter());
	}
}