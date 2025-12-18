#include <VadonDemo/Model/Enemy/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void EnemyBase::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyBase>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyBase>("definition", Vadon::Utilities::MemberVariableBind<&EnemyBase::definition>().bind_member_getter().bind_member_setter());
	}

	void EnemyMovement::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyMovement>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyMovement>("definition", Vadon::Utilities::MemberVariableBind<&EnemyMovement::definition>().bind_member_getter().bind_member_setter());
	}

	void EnemyWeapon::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyWeapon>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyWeapon>("definition", Vadon::Utilities::MemberVariableBind<&EnemyWeapon::definition>().bind_member_getter().bind_member_setter());
	}

	void EnemyContactDamage::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyContactDamage>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyContactDamage>("definition", Vadon::Utilities::MemberVariableBind<&EnemyContactDamage::definition>().bind_member_getter().bind_member_setter());
	}

	void Spawner::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Spawner>();

		Vadon::Utilities::TypeRegistry::add_property<Spawner>("enemy_prefab", Vadon::Utilities::MemberVariableBind<&Spawner::enemy_prefab>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("activation_delay", Vadon::Utilities::MemberVariableBind<&Spawner::activation_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("min_spawn_delay", Vadon::Utilities::MemberVariableBind<&Spawner::min_spawn_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("start_spawn_count", Vadon::Utilities::MemberVariableBind<&Spawner::start_spawn_count>().bind_member_getter().bind_member_setter());
		//Vadon::Utilities::TypeRegistry::add_property<Spawner>("spawn_rate", Vadon::Utilities::MemberVariableBind<&Spawner::spawn_rate>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("level_multiplier", Vadon::Utilities::MemberVariableBind<&Spawner::level_multiplier>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("level_up_delay", Vadon::Utilities::MemberVariableBind<&Spawner::level_up_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>("max_level", Vadon::Utilities::MemberVariableBind<&Spawner::max_level>().bind_member_getter().bind_member_setter());
	}
}