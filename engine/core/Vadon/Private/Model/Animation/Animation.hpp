#ifndef VADON_PRIVATE_MODEL_ANIMATION_ANIMATION_HPP
#define VADON_PRIVATE_MODEL_ANIMATION_ANIMATION_HPP
#include <Vadon/Model/Animation/Animation.hpp>
namespace Vadon::Private::Model
{
	using AnimationKey = Vadon::Model::AnimationKey;

	using AnimationChannel = Vadon::Model::AnimationChannel;

	using AnimationIntChannel = Vadon::Model::AnimationIntChannel;
	using AnimationUintChannel = Vadon::Model::AnimationUintChannel;
	using AnimationFloatChannel = Vadon::Model::AnimationFloatChannel;
	using AnimationVector2Channel = Vadon::Model::AnimationVector2Channel;
	using AnimationVector2iChannel = Vadon::Model::AnimationVector2iChannel;
	using AnimationVector3Channel = Vadon::Model::AnimationVector3Channel;
	using AnimationVector3iChannel = Vadon::Model::AnimationVector3iChannel;
	using AnimationVector4Channel = Vadon::Model::AnimationVector4Channel;
	using AnimationColorRGBAChannel = Vadon::Model::AnimationColorRGBAChannel;

	using Animation = Vadon::Model::Animation;

	using AnimationID = Vadon::Model::AnimationID;
	using AnimationHandle = Vadon::Model::AnimationHandle;
}
#endif