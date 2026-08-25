#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void WeaponDefinition::register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using ResourceRegistry = Vadon::Model::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<WeaponDefinition, Vadon::Model::Resource>();

		TypeRegistry::add_property<WeaponDefinition>(VADON_GET_MEMBER_UUID(WeaponDefinition, projectile_prefab), Vadon::Utilities::MemberVariableBind<&WeaponDefinition::projectile_prefab>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<WeaponDefinition>(VADON_GET_MEMBER_UUID(WeaponDefinition, rate_of_fire), Vadon::Utilities::MemberVariableBind<&WeaponDefinition::rate_of_fire>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(WeaponDefinition))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::WeaponDefinition")
			.add_property(VADON_GET_MEMBER_UUID(WeaponDefinition, projectile_prefab))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Projectile Prefab")
				.commit_property()
			.add_property(VADON_GET_MEMBER_UUID(WeaponDefinition, rate_of_fire))
				.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Rate of Fire")
				.commit_property();
	}
}