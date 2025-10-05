#ifndef VADON_PRIVATE_SCENE_ANIMATION_ANIMATION_HPP
#define VADON_PRIVATE_SCENE_ANIMATION_ANIMATION_HPP
#include <Vadon/Scene/Animation/Animation.hpp>
namespace Vadon::Private::Scene
{
	using AnimationKeyframe = Vadon::Scene::AnimationKeyframe;
	using AnimationChannel = Vadon::Scene::AnimationChannel;
	using AnimationData = Vadon::Scene::AnimationData;

	using AnimationID = Vadon::Scene::AnimationID;
	using AnimationHandle = Vadon::Scene::AnimationHandle;
}
namespace Vadon::Scene
{
	struct Animation : public Resource
	{
		AnimationData data;

		static void register_type_info();
	};
}
#endif