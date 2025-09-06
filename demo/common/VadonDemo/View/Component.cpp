#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void TransformComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<TransformComponent>();

		TypeRegistry::add_property<TransformComponent>("position", Vadon::Utilities::MemberVariableBind<&TransformComponent::position>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<TransformComponent>("rotation", Vadon::Utilities::MemberVariableBind<&TransformComponent::rotation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<TransformComponent>("scale", Vadon::Utilities::MemberVariableBind<&TransformComponent::scale>().bind_member_getter().bind_member_setter());
	}

	void ModelTransformComponent::register_component()
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ModelTransformComponent>();
	}

	void RenderComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<RenderComponent>();

		TypeRegistry::add_property<RenderComponent>("resource", Vadon::Utilities::MemberVariableBind<&RenderComponent::resource>().bind_member_getter().bind_member_setter());
	}

	void AnimationComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<AnimationComponent>();

		TypeRegistry::add_property<AnimationComponent>("time_scale", Vadon::Utilities::MemberVariableBind<&AnimationComponent::time_scale>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationComponent>("looping", Vadon::Utilities::MemberVariableBind<&AnimationComponent::looping>().bind_member_getter().bind_member_setter());
	}

	void DamageComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<DamageComponent>();

		TypeRegistry::add_property<DamageComponent>("animation", Vadon::Utilities::MemberVariableBind<&DamageComponent::animation>().bind_member_getter().bind_member_setter());
	}

	void VFXComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<VFXComponent>();

		TypeRegistry::add_property<VFXComponent>("vfx_prefab", Vadon::Utilities::MemberVariableBind<&VFXComponent::vfx_prefab>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>("animation", Vadon::Utilities::MemberVariableBind<&VFXComponent::animation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>("lifetime", Vadon::Utilities::MemberVariableBind<&VFXComponent::lifetime>().bind_member_getter().bind_member_setter());
	}

	void VFXTimerComponent::register_component()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::ComponentTypeInfo component_info;
		component_info.hint_string = "VadonEditor:exclude";

		Vadon::ECS::ComponentRegistry::register_component_type<VFXTimerComponent>(component_info);
	}
}