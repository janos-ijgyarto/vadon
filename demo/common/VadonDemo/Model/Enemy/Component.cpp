#include <VadonDemo/Model/Enemy/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void EnemyBase::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyBase>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyBase>(VADON_GET_MEMBER_UUID(EnemyBase, definition), Vadon::Utilities::MemberVariableBind<&EnemyBase::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata<EnemyBase> enemy_metadata(metadata_registry);
		enemy_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyBase");
		enemy_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		enemy_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyBase, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition");
	}

	void EnemyMovement::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyMovement>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyMovement>(VADON_GET_MEMBER_UUID(EnemyMovement, definition), Vadon::Utilities::MemberVariableBind<&EnemyMovement::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata<EnemyMovement> movement_metadata(metadata_registry);
		movement_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyMovement");
		movement_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		movement_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyMovement, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition");
	}

	void EnemyWeapon::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyWeapon>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyWeapon>(VADON_GET_MEMBER_UUID(EnemyWeapon, definition), Vadon::Utilities::MemberVariableBind<&EnemyWeapon::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata<EnemyWeapon> weapon_metadata(metadata_registry);
		weapon_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyWeapon");
		weapon_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		weapon_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyWeapon, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition");
	}

	void EnemyContactDamage::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyContactDamage>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyContactDamage>(VADON_GET_MEMBER_UUID(EnemyContactDamage, definition), Vadon::Utilities::MemberVariableBind<&EnemyContactDamage::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata<EnemyContactDamage> contact_damage_metadata(metadata_registry);
		contact_damage_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyContactDamage");
		contact_damage_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		contact_damage_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyContactDamage, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition");
	}

	void Spawner::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Spawner>();

		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, enemy_prefab), Vadon::Utilities::MemberVariableBind<&Spawner::enemy_prefab>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, activation_delay), Vadon::Utilities::MemberVariableBind<&Spawner::activation_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, min_spawn_delay), Vadon::Utilities::MemberVariableBind<&Spawner::min_spawn_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, start_spawn_count), Vadon::Utilities::MemberVariableBind<&Spawner::start_spawn_count>().bind_member_getter().bind_member_setter());
		//Vadon::Utilities::TypeRegistry::add_property<Spawner>("spawn_rate", Vadon::Utilities::MemberVariableBind<&Spawner::spawn_rate>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, level_multiplier), Vadon::Utilities::MemberVariableBind<&Spawner::level_multiplier>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, level_up_delay), Vadon::Utilities::MemberVariableBind<&Spawner::level_up_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Spawner>(VADON_GET_MEMBER_UUID(Spawner, max_level), Vadon::Utilities::MemberVariableBind<&Spawner::max_level>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata<Spawner> spawner_metadata(metadata_registry);
		spawner_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Spawner");
		spawner_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, enemy_prefab))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Enemy Prefab");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, activation_delay))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Activation Delay");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, min_spawn_delay))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Min Spawn Delay");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, start_spawn_count))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Start Spawn Count");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, level_multiplier))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Level Up Multiplier");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, level_up_delay))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Level Up Delay");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, max_level))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Max Level");
	}
}