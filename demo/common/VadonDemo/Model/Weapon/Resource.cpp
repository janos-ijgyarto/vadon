#include <VadonDemo/Model/Weapon/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void WeaponDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<WeaponDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<WeaponDefinition>("projectile_prefab", Vadon::Utilities::MemberVariableBind<&WeaponDefinition::projectile_prefab>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<WeaponDefinition>("rate_of_fire", Vadon::Utilities::MemberVariableBind<&WeaponDefinition::rate_of_fire>().bind_member_getter().bind_member_setter());
	}
}