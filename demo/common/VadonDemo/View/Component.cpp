#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void ViewComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewComponent>();

		TypeRegistry::add_property<ViewComponent>("resource", Vadon::Utilities::MemberVariableBind<&ViewComponent::resource>().bind_member_getter().bind_member_setter());
	}

	void ViewAnimationComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewAnimationComponent>();
	}

	void ViewDamageComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewDamageComponent>();

		TypeRegistry::add_property<ViewDamageComponent>("animation", Vadon::Utilities::MemberVariableBind<&ViewDamageComponent::animation>().bind_member_getter().bind_member_setter());
	}

	void ViewVFXComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<ViewVFXComponent>();

		TypeRegistry::add_property<ViewVFXComponent>("animation", Vadon::Utilities::MemberVariableBind<&ViewVFXComponent::animation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<ViewVFXComponent>("lifetime", Vadon::Utilities::MemberVariableBind<&ViewVFXComponent::lifetime>().bind_member_getter().bind_member_setter());
	}
}