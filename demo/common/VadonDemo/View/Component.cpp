#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewComponent>();

		TypeRegistry::add_property<ViewComponent>("resource", Vadon::Utilities::MemberVariableBind<&ViewComponent::resource>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<ViewComponent>("z_order", Vadon::Utilities::MemberVariableBind<&ViewComponent::z_order>().bind_member_getter().bind_member_setter());
	}
}