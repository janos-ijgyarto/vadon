#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Scene/Scene.hpp>
#include <Vadon/Scene/Animation/AnimationPlayer.hpp>
namespace VadonDemo::View
{
	// Tag to ensure View data is updated once properties change
	struct EntityDirtyTag {};

	// NOTE: this is the actual "ground truth" transform for a View entity
	// The other component expects to interpolate a model transform, and will set the result here
	struct TransformComponent
	{
		Vadon::Math::Vector2 position = Vadon::Math::Vector2_Zero;
		float rotation = 0;
		float scale = 1.0f;

		static void register_component();
	};

	// Interpolates model transform (if present)
	struct ModelTransformComponent
	{
		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		static void register_component();
	};

	struct RenderComponent
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		RenderResourceID resource;
		Vadon::Math::ColorRGBA color = Vadon::Math::Color_White;

		static void register_component();
	};

	struct AnimationComponent
	{
		float time_scale = 1.0f;
		bool looping = false;

		Vadon::Scene::AnimationPlayer animation_player;

		static void register_component();
	};

	// Visualizes when the entity takes damage
	struct DamageComponent
	{
		Vadon::Scene::AnimationID animation;
		float lifetime = 1.0f;

		static void register_component();
	};

	// TODO: create specific VFX components for specific contexts
	// For now, we'll just check what components the model has and implement behavior based on that
	struct VFXComponent
	{
		Vadon::Scene::SceneID vfx_prefab;
		Vadon::Scene::AnimationID animation;
		float lifetime = 1.0f;

		static void register_component();
	};

	struct VFXTimerComponent
	{
		float remaining_lifetime = 1.0f;

		static void register_component();
	};
}
#endif