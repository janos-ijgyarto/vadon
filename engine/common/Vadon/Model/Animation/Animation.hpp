#ifndef VADON_MODEL_ANIMATION_ANIMATION_HPP
#define VADON_MODEL_ANIMATION_ANIMATION_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Model/Resource/Resource.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
namespace Vadon::Model
{
	struct AnimationKeyframe
	{
		int32_t frame_index = 0;
		Vadon::Utilities::Variant value;
		// TODO: anything else?
	};

	enum class AnimationChannelType
	{
		INT,
		UINT32,
		FLOAT,
		VEC2,
		VEC2I,
		VEC3,
		VEC3I,
		VEC4,
		COLORRGBA,
		TYPE_COUNT,
		INVALID = TYPE_COUNT
	};

	struct AnimationChannel
	{
		// Used as metadata by other systems to apply the animation data
		std::string tag; // FIXME: replace with a UUID?
		AnimationChannelType data_type = AnimationChannelType::INVALID;
		Vadon::Utilities::DataRange keyframe_range;

		VADONCOMMON_API static Vadon::Utilities::TypeID get_data_type_id(AnimationChannelType channel_type);
	};

	struct AnimationData
	{
		int32_t frame_count = 0;
		std::vector<AnimationChannel> channels;
		std::vector<AnimationKeyframe> keyframe_data;
	};

	struct Animation;

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(Animation, AnimationID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(Animation, AnimationHandle);
}

VADON_REGISTER_TYPE_UUID(Vadon::Model::Animation, "8b41bdad-0036-45d2-ac31-81729abe6c25");
#endif