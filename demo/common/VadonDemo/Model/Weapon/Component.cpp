#include <VadonDemo/Model/Weapon/Component.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void WeaponComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<WeaponComponent>();

		Vadon::Utilities::TypeRegistry::add_property<WeaponComponent>(VADON_GET_MEMBER_UUID(WeaponComponent, definition), Vadon::Utilities::MemberVariableBind<&WeaponComponent::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata weapon_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(WeaponComponent));
		weapon_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::WeaponComponent");

		{
			Vadon::Utilities::TypePropertyMetadata definition_property(weapon_component_metadata, VADON_GET_MEMBER_UUID(WeaponComponent, definition));
			definition_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition");
			definition_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(WeaponDefinition));
		}
	}

	void WeaponVolleyComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<WeaponVolleyComponent>();

		Vadon::Utilities::TypeRegistry::add_property<WeaponVolleyComponent>(VADON_GET_MEMBER_UUID(WeaponVolleyComponent, fire_count), Vadon::Utilities::MemberVariableBind<&WeaponVolleyComponent::fire_count>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata volley_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(WeaponVolleyComponent));
		volley_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::WeaponVolleyComponent");

		{
			Vadon::Utilities::TypePropertyMetadata fire_count_property(volley_component_metadata, VADON_GET_MEMBER_UUID(WeaponVolleyComponent, fire_count));
			fire_count_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Fire Count");
		}
	}

	void ProjectileComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, range), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::range>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, damage), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::damage>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileComponent>(VADON_GET_MEMBER_UUID(ProjectileComponent, knockback), Vadon::Utilities::MemberVariableBind<&ProjectileComponent::knockback>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata projectile_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(ProjectileComponent));
		projectile_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::ProjectileComponent");

		{
			Vadon::Utilities::TypePropertyMetadata range_property(projectile_component_metadata, VADON_GET_MEMBER_UUID(ProjectileComponent, range));
			range_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Range");
		}
		{
			Vadon::Utilities::TypePropertyMetadata damage_property(projectile_component_metadata, VADON_GET_MEMBER_UUID(ProjectileComponent, damage));
			damage_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Damage");
		}
		{
			Vadon::Utilities::TypePropertyMetadata knockback_property(projectile_component_metadata, VADON_GET_MEMBER_UUID(ProjectileComponent, knockback));
			knockback_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Knockback");
		}
	}

	void ProjectileHomingComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileHomingComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>(VADON_GET_MEMBER_UUID(ProjectileHomingComponent, turn_speed), Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::turn_speed>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<ProjectileHomingComponent>(VADON_GET_MEMBER_UUID(ProjectileHomingComponent, aimed_on_launch), Vadon::Utilities::MemberVariableBind<&ProjectileHomingComponent::aimed_on_launch>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata homing_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(ProjectileHomingComponent));
		homing_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::ProjectileHomingComponent");

		{
			Vadon::Utilities::TypePropertyMetadata turn_speed_property(homing_component_metadata, VADON_GET_MEMBER_UUID(ProjectileHomingComponent, turn_speed));
			turn_speed_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Turn Speed");
		}

		{
			Vadon::Utilities::TypePropertyMetadata aimed_on_launch_property(homing_component_metadata, VADON_GET_MEMBER_UUID(ProjectileHomingComponent, aimed_on_launch));
			aimed_on_launch_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Aimed on Launch");
		}
	}

	void ProjectileAOEComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ProjectileAOEComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ProjectileAOEComponent>(VADON_GET_MEMBER_UUID(ProjectileAOEComponent, radius), Vadon::Utilities::MemberVariableBind<&ProjectileAOEComponent::radius>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata aoe_component_metadata(metadata_registry, VADON_GET_TYPE_UUID(ProjectileAOEComponent));
		aoe_component_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::ProjectileAOEComponent");

		{
			Vadon::Utilities::TypePropertyMetadata radius_property(aoe_component_metadata, VADON_GET_MEMBER_UUID(ProjectileAOEComponent, radius));
			radius_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Radius");
		}
	}
}