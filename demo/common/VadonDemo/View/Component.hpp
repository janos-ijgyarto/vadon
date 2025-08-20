#ifndef VADONDEMO_VIEW_COMPONENT_HPP
#define VADONDEMO_VIEW_COMPONENT_HPP
#include <VadonDemo/View/Resource.hpp>
#include <Vadon/Scene/Animation/AnimationPlayer.hpp>
namespace VadonDemo::View
{
	// Tag to ensure View data is updated once properties change
	struct ViewEntityDirtyTag {};

	struct ViewComponent
	{
		// TODO: implement a utility type which takes care of both the persistent resource ID and the loaded resource handle?
		ViewResourceID resource;

		Vadon::Render::Canvas::Transform prev_transform;
		Vadon::Render::Canvas::Transform current_transform;

		static void register_component();
	};

	struct ViewAnimationComponent
	{
		Vadon::Scene::AnimationPlayer animation_player;

		static void register_component();
	};

	// Visualizes when the entity takes damage
	struct ViewDamageComponent
	{
		Vadon::Scene::AnimationID animation;
		float lifetime = 1.0f;

		static void register_component();
	};

	// TODO: implement animation system?
	struct ViewVFXComponent
	{
		Vadon::Scene::AnimationID animation;
		float lifetime = 1.0f;

		static void register_component();
	};
}
#endif