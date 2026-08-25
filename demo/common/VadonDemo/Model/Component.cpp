#include <VadonDemo/Model/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void Transform2D::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Transform2D>();
				
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>(VADON_GET_MEMBER_UUID(Transform2D, position), Vadon::Utilities::MemberVariableBind<&Transform2D::position>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>(VADON_GET_MEMBER_UUID(Transform2D, rotation), Vadon::Utilities::MemberVariableBind<&Transform2D::rotation>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>(VADON_GET_MEMBER_UUID(Transform2D, scale), Vadon::Utilities::MemberVariableBind<&Transform2D::scale>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata transform_metadata(metadata_registry, VADON_GET_TYPE_UUID(Transform2D));
		transform_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Transform2D");
		transform_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(Transform2D, position))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Position");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(Transform2D, rotation))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Rotation");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(Transform2D, scale))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Scale");
	}

	void Velocity2D::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Velocity2D>();

		Vadon::Utilities::TypeRegistry::add_property<Velocity2D>(VADON_GET_MEMBER_UUID(Velocity2D, top_speed), Vadon::Utilities::MemberVariableBind<&Velocity2D::top_speed>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Velocity2D>(VADON_GET_MEMBER_UUID(Velocity2D, acceleration), Vadon::Utilities::MemberVariableBind<&Velocity2D::acceleration>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata velocity_metadata(metadata_registry, VADON_GET_TYPE_UUID(Velocity2D));
		velocity_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Velocity2D");
		velocity_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		{
			Vadon::Utilities::TypePropertyMetadata top_speed_property(velocity_metadata, VADON_GET_MEMBER_UUID(Velocity2D, top_speed));
			top_speed_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Top Speed");
		}

		{
			Vadon::Utilities::TypePropertyMetadata acceleration_property(velocity_metadata, VADON_GET_MEMBER_UUID(Velocity2D, acceleration));
			acceleration_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Acceleration");
		}
	}

	void Health::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Health>();

		Vadon::Utilities::TypeRegistry::add_property<Health>(VADON_GET_MEMBER_UUID(Health, max_health), Vadon::Utilities::MemberVariableBind<&Health::max_health>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata health_metadata(metadata_registry, VADON_GET_TYPE_UUID(Health));
		health_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Health");
		health_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		{
			Vadon::Utilities::TypePropertyMetadata max_health_property(health_metadata, VADON_GET_MEMBER_UUID(Health, max_health));
			max_health_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Max Health");
		}
	}

	void Player::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Player>();

		Vadon::Utilities::TypeRegistry::add_property<Player>(VADON_GET_MEMBER_UUID(Player, damage_delay), Vadon::Utilities::MemberVariableBind<&Player::damage_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Player>(VADON_GET_MEMBER_UUID(Player, starting_weapons), Vadon::Utilities::MemberVariableBind<&Player::starting_weapons>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata player_metadata(metadata_registry, VADON_GET_TYPE_UUID(Player));
		player_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Player");
		player_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		{
			Vadon::Utilities::TypePropertyMetadata damage_delay_property(player_metadata, VADON_GET_MEMBER_UUID(Player, damage_delay));
			damage_delay_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Damage Delay");
		}

		{
			Vadon::Utilities::TypePropertyMetadata starting_weapons_property(player_metadata, VADON_GET_MEMBER_UUID(Player, starting_weapons));
			starting_weapons_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Starting Weapons");
		}
	}

	void Map::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Map>();

		Vadon::Utilities::TypeRegistry::add_property<Map>(VADON_GET_MEMBER_UUID(Map, display_name), Vadon::Utilities::MemberVariableBind<&Map::display_name>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Map>(VADON_GET_MEMBER_UUID(Map, dimensions), Vadon::Utilities::MemberVariableBind<&Map::dimensions>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata map_metadata(metadata_registry, VADON_GET_TYPE_UUID(Map));
		map_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::Model::Map");
		map_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "VadonDemo/Model");

		{
			Vadon::Utilities::TypePropertyMetadata display_name_property(map_metadata, VADON_GET_MEMBER_UUID(Map, display_name));
			display_name_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Display Name");
		}

		{
			Vadon::Utilities::TypePropertyMetadata dimensions_property(map_metadata, VADON_GET_MEMBER_UUID(Map, dimensions));
			dimensions_property.set_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Dimensions");
		}
	}
}