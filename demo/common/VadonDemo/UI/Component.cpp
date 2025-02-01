#include <VadonDemo/UI/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::UI
{
	void Base::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Base>();

		Vadon::Utilities::TypeRegistry::add_property<Base>("visible", Vadon::Utilities::MemberVariableBind<&Base::visible>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Base>("position", Vadon::Utilities::MemberVariableBind<&Base::position>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Base>("dimensions", Vadon::Utilities::MemberVariableBind<&Base::dimensions>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Base>("z_order", Vadon::Utilities::MemberVariableBind<&Base::z_order>().bind_member_getter().bind_member_setter());
	}

	void Frame::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Frame>();

		Vadon::Utilities::TypeRegistry::add_property<Frame>("outline_color", Vadon::Utilities::MemberVariableBind<&Frame::outline_color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>("outline_thickness", Vadon::Utilities::MemberVariableBind<&Frame::outline_thickness>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>("fill_color", Vadon::Utilities::MemberVariableBind<&Frame::fill_color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Frame>("fill_enable", Vadon::Utilities::MemberVariableBind<&Frame::fill_enable>().bind_member_getter().bind_member_setter());
	}

	void Text::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Text>();

		Vadon::Utilities::TypeRegistry::add_property<Text>("text", Vadon::Utilities::MemberVariableBind<&Text::text>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Text>("color", Vadon::Utilities::MemberVariableBind<&Text::color>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<Text>("offset", Vadon::Utilities::MemberVariableBind<&Text::offset>().bind_member_getter().bind_member_setter());
	}

	void Selectable::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<Selectable>();
	}
}