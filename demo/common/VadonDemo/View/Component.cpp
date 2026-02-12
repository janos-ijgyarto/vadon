#include <VadonDemo/View/Component.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

namespace VadonDemo::View
{
	void TransformComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<TransformComponent>();

		TypeRegistry::add_property<TransformComponent>(VADON_GET_MEMBER_UUID(TransformComponent, position), Vadon::Utilities::MemberVariableBind<&TransformComponent::position>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<TransformComponent>(VADON_GET_MEMBER_UUID(TransformComponent, rotation), Vadon::Utilities::MemberVariableBind<&TransformComponent::rotation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<TransformComponent>(VADON_GET_MEMBER_UUID(TransformComponent, scale), Vadon::Utilities::MemberVariableBind<&TransformComponent::scale>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata transform_metadata(metadata_registry, VADON_GET_TYPE_UUID(TransformComponent));
		transform_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::TransformComponent");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(TransformComponent, position))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Position");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(TransformComponent, rotation))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Rotation");

		transform_metadata.add_property(VADON_GET_MEMBER_UUID(TransformComponent, scale))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Scale");
	}

	void ModelTransformComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		Vadon::ECS::ComponentRegistry::register_component_type<ModelTransformComponent>();

		Vadon::Utilities::TypeMetadata transform_metadata(metadata_registry, VADON_GET_TYPE_UUID(ModelTransformComponent));
		transform_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::ModelTransformComponent");
	}

	void RenderComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<RenderComponent>();

		TypeRegistry::add_property<RenderComponent>(VADON_GET_MEMBER_UUID(RenderComponent, resource), Vadon::Utilities::MemberVariableBind<&RenderComponent::resource>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<RenderComponent>(VADON_GET_MEMBER_UUID(RenderComponent, color), Vadon::Utilities::MemberVariableBind<&RenderComponent::color>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata render_metadata(metadata_registry, VADON_GET_TYPE_UUID(RenderComponent));
		render_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::RenderComponent");

		render_metadata.add_property(VADON_GET_MEMBER_UUID(RenderComponent, resource))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Resource")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(RenderResource));

		render_metadata.add_property(VADON_GET_MEMBER_UUID(RenderComponent, color))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Color");
	}

	void AnimationComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<AnimationComponent>();

		TypeRegistry::add_property<AnimationComponent>(VADON_GET_MEMBER_UUID(AnimationComponent, time_scale), Vadon::Utilities::MemberVariableBind<&AnimationComponent::time_scale>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationComponent>(VADON_GET_MEMBER_UUID(AnimationComponent, looping), Vadon::Utilities::MemberVariableBind<&AnimationComponent::looping>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata animation_metadata(metadata_registry, VADON_GET_TYPE_UUID(AnimationComponent));
		animation_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::AnimationComponent");

		animation_metadata.add_property(VADON_GET_MEMBER_UUID(AnimationComponent, time_scale))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Time Scale");

		animation_metadata.add_property(VADON_GET_MEMBER_UUID(AnimationComponent, looping))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Looping");
	}

	void DamageComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<DamageComponent>();

		TypeRegistry::add_property<DamageComponent>(VADON_GET_MEMBER_UUID(DamageComponent, animation), Vadon::Utilities::MemberVariableBind<&DamageComponent::animation>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata damage_metadata(metadata_registry, VADON_GET_TYPE_UUID(DamageComponent));
		damage_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::DamageComponent");

		damage_metadata.add_property(VADON_GET_MEMBER_UUID(DamageComponent, animation))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Animation")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(Vadon::Scene::Animation));
	}

	void VFXComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<VFXComponent>();

		TypeRegistry::add_property<VFXComponent>(VADON_GET_MEMBER_UUID(VFXComponent, vfx_prefab), Vadon::Utilities::MemberVariableBind<&VFXComponent::vfx_prefab>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>(VADON_GET_MEMBER_UUID(VFXComponent, animation), Vadon::Utilities::MemberVariableBind<&VFXComponent::animation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<VFXComponent>(VADON_GET_MEMBER_UUID(VFXComponent, lifetime), Vadon::Utilities::MemberVariableBind<&VFXComponent::lifetime>().bind_member_getter().bind_member_setter());

		Vadon::Utilities::TypeMetadata vfx_metadata(metadata_registry, VADON_GET_TYPE_UUID(VFXComponent));
		vfx_metadata.set_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::VFXComponent");

		vfx_metadata.add_property(VADON_GET_MEMBER_UUID(VFXComponent, vfx_prefab))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "VFX Prefab")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(Vadon::Scene::Scene));

		vfx_metadata.add_property(VADON_GET_MEMBER_UUID(VFXComponent, animation))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Animation")
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::RESOURCE_TYPE, VADON_GET_TYPE_UUID_BASE64_STRING(Vadon::Scene::Animation));

		vfx_metadata.add_property(VADON_GET_MEMBER_UUID(VFXComponent, lifetime))
			.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Lifetime");
	}

	void VFXTimerComponent::register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		Vadon::ECS::ComponentRegistry::register_component_type<VFXTimerComponent>();

		Vadon::Utilities::TypeMetadata(metadata_registry, VADON_GET_TYPE_UUID(VFXTimerComponent))
			.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "VadonDemo::View::VFXTimerComponent");
	}
}