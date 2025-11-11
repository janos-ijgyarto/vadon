#include <VadonDemo/Model/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::Model
{
	void Transform2D::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Transform2D>();
				
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("position", Vadon::Utilities::MemberVariableBind<&Transform2D::position>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("rotation", Vadon::Utilities::MemberVariableBind<&Transform2D::rotation>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("scale", Vadon::Utilities::MemberVariableBind<&Transform2D::scale>().bind_member_getter().bind_member_setter());
	}

	void Velocity2D::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Velocity2D>();

		Vadon::Utilities::TypeRegistry::add_property<Velocity2D>("top_speed", Vadon::Utilities::MemberVariableBind<&Velocity2D::top_speed>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Velocity2D>("acceleration", Vadon::Utilities::MemberVariableBind<&Velocity2D::acceleration>().bind_member_getter().bind_member_setter());
	}

	void Health::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Health>();

		Vadon::Utilities::TypeRegistry::add_property<Health>("max_health", Vadon::Utilities::MemberVariableBind<&Health::max_health>().bind_member_getter().bind_member_setter());
	}

	void Player::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Player>();

		Vadon::Utilities::TypeRegistry::add_property<Player>("damage_delay", Vadon::Utilities::MemberVariableBind<&Player::damage_delay>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Player>("starting_weapons", Vadon::Utilities::MemberVariableBind<&Player::starting_weapons>().bind_member_getter().bind_member_setter());
	}

	void Map::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Map>();

		Vadon::Utilities::TypeRegistry::add_property<Map>("display_name", Vadon::Utilities::MemberVariableBind<&Map::display_name>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Map>("dimensions", Vadon::Utilities::MemberVariableBind<&Map::dimensions>().bind_member_getter().bind_member_setter());
	}
}