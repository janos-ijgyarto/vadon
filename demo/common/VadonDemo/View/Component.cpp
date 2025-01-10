#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ViewComponent>();

		Vadon::Utilities::TypeRegistry::add_property<ViewComponent>("resource", Vadon::Utilities::MemberVariableBind<&ViewComponent::resource>().bind_member_getter().bind_member_setter());
	}
}