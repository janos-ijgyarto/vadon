#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Scene/Animation/AnimationPlayer.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::View
{
	// Tag to ensure View data is updated once properties change
	struct EntityDirtyTag {};

	// NOTE: this is the actual "ground truth" transform for a View entity
	// The other component expects to interpolate a model transform, and will set the result here
	struct TransformComponent
	{
		VADON_DECLARE_MEMBER_UUID(position, "a27816bb-49c8-46d5-b9c0-d0863ec477fc");
		VADON_DECLARE_MEMBER_UUID(rotation, "3b3634eb-813a-4bf3-8927-35b23fc62b9c");
		VADON_DECLARE_MEMBER_UUID(scale, "96f49120-d977-4934-8559-086529b6d7f9");

		Vadon::Math::Vector2 position = Vadon::Math::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	// Interpolates model transform (if present)
	struct ModelTransformComponent
	{
		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct RenderComponent
	{
		VADON_DECLARE_MEMBER_UUID(resource, "eec84b74-8dcb-4bab-b675-7402d5e78984");
		VADON_DECLARE_MEMBER_UUID(color, "1a5ae5fd-95ea-4fa0-9e3e-15d6ab163084");

		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		RenderResourceID resource;
		Vadon::Math::ColorRGBA color = Vadon::Math::Color_White;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct AnimationComponent
	{
		VADON_DECLARE_MEMBER_UUID(time_scale, "7b9d82e1-769d-46c3-a6b3-786f119c6321");
		VADON_DECLARE_MEMBER_UUID(looping, "67e6ec9f-6c81-4ddf-b905-dadad9482753");

		float time_scale = 1.0f;
		bool looping = false;

		Vadon::Scene::AnimationPlayer animation_player;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	// Visualizes when the entity takes damage
	struct DamageComponent
	{
		VADON_DECLARE_MEMBER_UUID(animation, "3834c3fa-6027-4c90-a1ea-65f50eaaca67");

		Vadon::Scene::AnimationID animation;
		float duration = 1.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	// TODO: create specific VFX components for specific contexts
	// For now, we'll just check what components the model has and implement behavior based on that
	struct VFXComponent
	{
		VADON_DECLARE_MEMBER_UUID(vfx_prefab, "3e712271-dacd-46fa-a4a5-959dbf505962");
		VADON_DECLARE_MEMBER_UUID(animation, "475cc76f-307d-4ba4-b116-87933c7b7c9a");
		VADON_DECLARE_MEMBER_UUID(lifetime, "d91706ae-4900-4e7d-9479-ceee4bceaa72");

		Vadon::Scene::SceneID vfx_prefab;
		Vadon::Scene::AnimationID animation;
		float lifetime = 1.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	struct VFXTimerComponent
	{
		float remaining_lifetime = 1.0f;

		static void register_component(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
}

VADON_REGISTER_TYPE_UUID(VadonDemo::View::EntityDirtyTag, "16fbbc81-5e5a-4b0f-a2cc-dc676047f8ec");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::TransformComponent, "ea3323ac-0682-48bb-93a6-d77d2ac87910");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::ModelTransformComponent, "dc22d853-fc16-4e04-9e08-eb1ad68a6ac3");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::RenderComponent, "b236a552-5ef1-4576-a8d4-b755b212a165");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::AnimationComponent, "14fbc013-0bd0-4f69-9c95-d805fe40c967");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::DamageComponent, "1d7bd96d-2b6b-459b-bd9f-ff093142b578");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::VFXComponent, "6e26c888-4bf9-4006-9f34-ea9f746162b1");
VADON_REGISTER_TYPE_UUID(VadonDemo::View::VFXTimerComponent, "0de92f74-4943-4d55-94d4-fd44d22a2d69");
#endif