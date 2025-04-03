#include <VadonDemo/Core/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::Core
{
	void CoreComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<CoreComponent>();

		Vadon::Utilities::TypeRegistry::add_property<CoreComponent>("main_menu", Vadon::Utilities::MemberVariableBind<&CoreComponent::main_menu>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<CoreComponent>("default_start_level", Vadon::Utilities::MemberVariableBind<&CoreComponent::default_start_level>().bind_member_getter().bind_member_setter());
		Vadon::Utilities::TypeRegistry::add_property<CoreComponent>("viewport_size", Vadon::Utilities::MemberVariableBind<&CoreComponent::viewport_size>().bind_member_getter().bind_member_setter());
	}
}