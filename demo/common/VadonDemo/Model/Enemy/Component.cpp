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

		Vadon::Utilities::TypeMetadata enemy_metadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyBase));
		enemy_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyBase");
		enemy_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		enemy_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyBase, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(EnemyDefinition));
	}

	void EnemyMovement::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyMovement>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyMovement>(VADON_GET_MEMBER_UUID(EnemyMovement, definition), Vadon::Utilities::MemberVariableBind<&EnemyMovement::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata movement_metadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyMovement));
		movement_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyMovement");
		movement_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		movement_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyMovement, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(EnemyMovementDefinition));
	}

	void EnemyWeapon::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyWeapon>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyWeapon>(VADON_GET_MEMBER_UUID(EnemyWeapon, definition), Vadon::Utilities::MemberVariableBind<&EnemyWeapon::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata weapon_metadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyWeapon));
		weapon_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyWeapon");
		weapon_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		weapon_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyWeapon, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(EnemyWeaponAttackDefinition));
	}

	void EnemyContactDamage::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<EnemyContactDamage>();

		Vadon::Utilities::TypeRegistry::add_property<EnemyContactDamage>(VADON_GET_MEMBER_UUID(EnemyContactDamage, definition), Vadon::Utilities::MemberVariableBind<&EnemyContactDamage::definition>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata contact_damage_metadata(metadata_registry, VADON_GET_TYPE_UUID(EnemyContactDamage));
		contact_damage_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::EnemyContactDamage");
		contact_damage_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		contact_damage_metadata.add_property(VADON_GET_MEMBER_UUID(EnemyContactDamage, definition))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Definition")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(EnemyContactDamageDefinition));
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

		Vadon::Utilities::TypeMetadata spawner_metadata(metadata_registry, VADON_GET_TYPE_UUID(Spawner));
		spawner_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Spawner");
		spawner_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		spawner_metadata.add_property(VADON_GET_MEMBER_UUID(Spawner, enemy_prefab))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Enemy Prefab")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(Vadon::Scene::Scene));

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