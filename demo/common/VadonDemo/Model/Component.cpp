#include <VadonDemo/Model/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

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

	void CanvasComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<CanvasComponent>();

		Vadon::Utilities::TypeRegistry::add_property<CanvasComponent>("type", Vadon::Utilities::MemberVariableBind<&CanvasComponent::type>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<CanvasComponent>("color", Vadon::Utilities::MemberVariableBind<&CanvasComponent::color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<CanvasComponent>("z_order", Vadon::Utilities::MemberVariableBind<&CanvasComponent::z_order>().bind_member_getter().bind_member_setter());
	}

	void Health::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Health>();

		Vadon::Utilities::TypeRegistry::add_property<Health>("max_health", Vadon::Utilities::MemberVariableBind<&Health::max_health>().bind_member_getter().bind_member_setter());
	}

	void Player::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Player>();
	}

	void Weapon::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Weapon>();

		Vadon::Utilities::TypeRegistry::add_property<Weapon>("projectile_prefab", Vadon::Utilities::MemberVariableBind<&Weapon::projectile_prefab>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Weapon>("rate_of_fire", Vadon::Utilities::MemberVariableBind<&Weapon::rate_of_fire>().bind_member_getter().bind_member_setter());
	}

	void Projectile::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Projectile>();

		Vadon::Utilities::TypeRegistry::add_property<Projectile>("range", Vadon::Utilities::MemberVariableBind<&Projectile::range>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Projectile>("damage", Vadon::Utilities::MemberVariableBind<&Projectile::damage>().bind_member_getter().bind_member_setter());
	}

	void Enemy::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Enemy>();

		Vadon::Utilities::TypeRegistry::add_property<Enemy>("score_reward", Vadon::Utilities::MemberVariableBind<&Enemy::score_reward>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Enemy>("damage", Vadon::Utilities::MemberVariableBind<&Enemy::damage>().bind_member_getter().bind_member_setter());
	}

	void Map::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Map>();

		Vadon::Utilities::TypeRegistry::add_property<Map>("display_name", Vadon::Utilities::MemberVariableBind<&Map::display_name>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Map>("dimensions", Vadon::Utilities::MemberVariableBind<&Map::dimensions>().bind_member_getter().bind_member_setter());
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