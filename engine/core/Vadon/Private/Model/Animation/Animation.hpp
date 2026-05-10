#ifndef VADON_PRIVATE_MODEL_ANIMATION_ANIMATION_HPP
#define VADON_PRIVATE_MODEL_ANIMATION_ANIMATION_HPP
#include <Vadon/Model/Animation/Animation.hpp>
namespace Vadon::Private::Model
{
	using AnimationKeyframe = Vadon::Model::AnimationKeyframe;
	using AnimationChannel = Vadon::Model::AnimationChannel;
	using AnimationData = Vadon::Model::AnimationData;

	using AnimationID = Vadon::Model::AnimationID;
	using AnimationHandle = Vadon::Model::AnimationHandle;
}
namespace Vadon::Model
{
	struct Animation : public Resource
	{
		AnimationData data;

		static void register_type_info();
	};
}
#endif