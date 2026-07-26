#ifndef VADON_MODEL_ANIMATION_ANIMATION_HPP
#define VADON_MODEL_ANIMATION_ANIMATION_HPP
#include <Vadon/Common.hpp>
#include <Vadon/Model/Resource/Resource.hpp>

#include <Vadon/Utilities/Data/Object.hpp>

#include <Vadon/Foundation/Model/Animation/Animation.hpp>
namespace Vadon::Model
{
	using AnimationChannelType = ::Vadon::Foundation::AnimationChannelSchema::Type;

	struct AnimationKey
	{
		float time = 0;
		float duration = 0; // TODO: implement support for keys that have duration (e.g audio clips)
		Vadon::Utilities::Variant value;
		// TODO: anything else?
	};

	struct AnimationChannel
	{
		VADON_DECLARE_MEMBER_UUID(id, ::Vadon::Foundation::AnimationChannelSchema::c_id_property.id.string);
		VADON_DECLARE_MEMBER_UUID(key_times, ::Vadon::Foundation::AnimationChannelSchema::c_key_times_property.id.string);
		VADON_DECLARE_MEMBER_UUID(tag, ::Vadon::Foundation::AnimationChannelSchema::c_tag_property.id.string);

		VADON_DECLARE_MEMBER_UUID(int_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::INT).string);
		VADON_DECLARE_MEMBER_UUID(uint_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::UINT).string);
		VADON_DECLARE_MEMBER_UUID(float_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::FLOAT).string);
		VADON_DECLARE_MEMBER_UUID(vector2_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::VECTOR2).string);
		VADON_DECLARE_MEMBER_UUID(vector2i_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::VECTOR2I).string);
		VADON_DECLARE_MEMBER_UUID(vector3_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::VECTOR3).string);
		VADON_DECLARE_MEMBER_UUID(vector3i_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::VECTOR3I).string);
		VADON_DECLARE_MEMBER_UUID(vector4_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::VECTOR4).string);
		VADON_DECLARE_MEMBER_UUID(colorrgba_data, ::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(Vadon::Model::AnimationChannelType::COLORRGBA).string);

		::Vadon::Foundation::UUID id;
		std::string tag; // FIXME: temporary solution, we should instead have users of the animation create "links" between channels and the affected properties
		std::vector<float> key_times;
		// TODO: key durations!

		virtual AnimationChannelType get_type() const { return AnimationChannelType::INVALID; }
		size_t get_key_count() const { return key_times.size(); }

		virtual Vadon::Utilities::Variant get_key_data(size_t index) const = 0;

		AnimationKey get_key(size_t index) const
		{
			return AnimationKey{ .time = key_times[index], .duration = 0.0f, .value = get_key_data(index) };
		}
	};

	template<typename T, AnimationChannelType C>
	struct AnimationChannelImpl : public AnimationChannel
	{
		std::vector<T> key_data;

		AnimationChannelType get_type() const override { return C; }
		Vadon::Utilities::Variant get_key_data(size_t index) const override { return key_data[index]; }
	};
}

#define VADON_DECLARE_ANIMATION_CHANNEL_TYPE(_type, _type_enum, _name) using _name = Vadon::Model::AnimationChannelImpl<_type, _type_enum>

namespace Vadon::Model
{
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(::Vadon::Foundation::int32, AnimationChannelType::INT, AnimationIntChannel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(::Vadon::Foundation::uint32, AnimationChannelType::UINT, AnimationUintChannel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(float, AnimationChannelType::FLOAT, AnimationFloatChannel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::Vector2, AnimationChannelType::VECTOR2, AnimationVector2Channel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::Vector2i, AnimationChannelType::VECTOR2I, AnimationVector2iChannel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::Vector3, AnimationChannelType::VECTOR3, AnimationVector3Channel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::Vector3i, AnimationChannelType::VECTOR3I, AnimationVector3iChannel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::Vector4, AnimationChannelType::VECTOR4, AnimationVector4Channel);
	VADON_DECLARE_ANIMATION_CHANNEL_TYPE(Vadon::Math::ColorRGBA, AnimationChannelType::COLORRGBA, AnimationColorRGBAChannel);

	using AnimationChannelWrapper = Vadon::Utilities::TypedObjectWrapper<AnimationChannel>;

	struct Animation : public Resource
	{
		VADON_DECLARE_MEMBER_UUID(channels, ::Vadon::Foundation::AnimationSchema::c_channels_property.id.string);

		std::vector<AnimationChannelWrapper> channels;

		~Animation()
		{
			clear_channels();
		}

		void set_channels(const std::vector<AnimationChannelWrapper>& channel_vec)
		{
			clear_channels();
			channels = channel_vec;
		}

		void clear_channels()
		{
			// We need to destroy each channel, since they were heap-allocated
			// FIXME: use refcounting and/or some kind of allocator strategy to make this more robust!
			for (const AnimationChannelWrapper& current_channel : channels)
			{
				Vadon::Utilities::TypeRegistry::destroy_object(current_channel);
			}

			channels.clear();
		}
	};

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(Animation, AnimationID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(Animation, AnimationHandle);
}

VADON_REGISTER_TYPE_UUID(Vadon::Model::Animation, ::Vadon::Foundation::AnimationSchema::c_type_uuid.string);

VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationChannel, ::Vadon::Foundation::AnimationChannelSchema::c_base_type_uuid.string);

VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationIntChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::INT).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationUintChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::UINT).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationFloatChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::FLOAT).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationVector2Channel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::VECTOR2).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationVector2iChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::VECTOR2I).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationVector3Channel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::VECTOR3).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationVector3iChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::VECTOR3I).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationVector4Channel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::VECTOR4).string);
VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationColorRGBAChannel, ::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(Vadon::Model::AnimationChannelType::COLORRGBA).string);

VADON_DEFINE_OBJECT_TYPE_ERASURE(Vadon::Model::AnimationChannel);
#endif