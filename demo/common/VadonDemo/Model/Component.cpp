#include <VadonDemo/Model/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

namespace VadonDemo::Model
{
	void Transform2D::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Transform2D>();
				
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("position", Vadon::Utilities::MemberVariableBind<&Transform2D::position>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("rotation", Vadon::Utilities::MemberVariableBind<&Transform2D::rotation>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Transform2D>("scale", Vadon::Utilities::MemberVariableBind<&Transform2D::scale>().bind_member_getter().bind_member_setter());
	}

	void CanvasComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<CanvasComponent>();

		Vadon::Utilities::TypeRegistry::add_property<CanvasComponent>("color", Vadon::Utilities::MemberVariableBind<&CanvasComponent::color>().bind_member_getter().bind_member_setter());
	}

	void Celestial::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Celestial>();

		Vadon::Utilities::TypeRegistry::add_property<Celestial>("is_star", Vadon::Utilities::MemberVariableBind<&Celestial::is_star>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Celestial>("angular_velocity", Vadon::Utilities::MemberVariableBind<&Celestial::angular_velocity>().bind_member_getter().bind_member_setter());
	}
}