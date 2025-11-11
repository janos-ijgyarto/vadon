#include <VadonDemo/Model/Enemy/Resource.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void EnemyDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<EnemyDefinition>("score_reward", Vadon::Utilities::MemberVariableBind<&EnemyDefinition::score_reward>().bind_member_getter().bind_member_setter());
	}

	void EnemyMovementDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyMovementDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<EnemyMovementDefinition>("movement_type", Vadon::Utilities::MemberVariableBind<&EnemyMovementDefinition::movement_type>().bind_member_getter().bind_member_setter());
	}

	void EnemyWeaponAttackDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;

		ResourceRegistry::register_resource_type<EnemyWeaponAttackDefinition, Vadon::Scene::Resource>();
	}

	void EnemyContactDamageDefinition::register_resource()
	{
		using ResourceRegistry = Vadon::Scene::ResourceRegistry;
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		ResourceRegistry::register_resource_type<EnemyContactDamageDefinition, Vadon::Scene::Resource>();

		TypeRegistry::add_property<EnemyContactDamageDefinition>("damage", Vadon::Utilities::MemberVariableBind<&EnemyContactDamageDefinition::damage>().bind_member_getter().bind_member_setter());
	}
}