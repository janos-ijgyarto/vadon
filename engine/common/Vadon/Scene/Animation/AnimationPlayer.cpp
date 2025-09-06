#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Scene/Animation/AnimationPlayer.hpp>

#include <Vadon/Scene/Animation/AnimationSystem.hpp>

#include <algorithm>

namespace
{
	Vadon::Utilities::Variant lerp_variant_values(const Vadon::Utilities::Variant& value_a, const Vadon::Utilities::Variant& value_b, float factor)
	{
		VADON_ASSERT(value_a.index() == value_b.index(), "Mismatch in variant types!");
		switch (value_a.index())
		{
		case Vadon::Utilities::type_list_index_v<int, Vadon::Utilities::Variant>:
			return std::lerp(float(std::get<int>(value_a)), float(std::get<int>(value_b)), factor);
		case Vadon::Utilities::type_list_index_v<uint32_t, Vadon::Utilities::Variant>:
			return std::lerp(float(std::get<uint32_t>(value_a)), float(std::get<uint32_t>(value_b)), factor);
		case Vadon::Utilities::type_list_index_v<float, Vadon::Utilities::Variant>:
			return std::lerp(std::get<float>(value_a), std::get<float>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2, Vadon::Utilities::Variant>:
			return Vadon::Math::Vector::mix(std::get<Vadon::Math::Vector2>(value_a), std::get<Vadon::Math::Vector2>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2i, Vadon::Utilities::Variant>:
			return Vadon::Math::Vector::mix(std::get<Vadon::Math::Vector2i>(value_a), std::get<Vadon::Math::Vector2i>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3, Vadon::Utilities::Variant>:
			return Vadon::Math::Vector::mix(std::get<Vadon::Math::Vector3>(value_a), std::get<Vadon::Math::Vector3>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3i, Vadon::Utilities::Variant>:
			return Vadon::Math::Vector::mix(std::get<Vadon::Math::Vector3i>(value_a), std::get<Vadon::Math::Vector3i>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::Vector4, Vadon::Utilities::Variant>:
			return Vadon::Math::Vector::mix(std::get<Vadon::Math::Vector4>(value_a), std::get<Vadon::Math::Vector4>(value_b), factor);
		case Vadon::Utilities::type_list_index_v<Vadon::Math::ColorRGBA, Vadon::Utilities::Variant>:
		{
			return Vadon::Math::ColorRGBA::from_rgba_vector(Vadon::Math::Vector::mix(Vadon::Math::ColorRGBA::to_rgba_vector(std::get<Vadon::Math::ColorRGBA>(value_a)),
				Vadon::Math::ColorRGBA::to_rgba_vector(std::get<Vadon::Math::ColorRGBA>(value_b)), factor));
		}
		default:
			break;
		}

		VADON_UNREACHABLE;
	}
}

namespace Vadon::Scene
{
	Vadon::Utilities::ErasedDataTypeID AnimationChannel::get_data_type_id(AnimationChannelType channel_type)
	{
		static constexpr uint32_t c_channel_type_map[static_cast<size_t>(AnimationChannelType::TYPE_COUNT)] = {
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<int, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<uint32_t, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<float, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::Vector2i, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::Vector3i, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::Vector4, Vadon::Utilities::Variant>),
			static_cast<uint32_t>(Vadon::Utilities::type_list_index_v<Vadon::Math::ColorRGBA, Vadon::Utilities::Variant>)
		};

		return Vadon::Utilities::ErasedDataTypeID{ .type = Vadon::Utilities::ErasedDataType::TRIVIAL, .id = c_channel_type_map[static_cast<size_t>(channel_type)] };
	}

	void AnimationPlayer::set_animation(Vadon::Core::EngineCoreInterface& engine_core, AnimationHandle animation_handle)
	{
		if (m_animation == animation_handle)
		{
			return;
		}

		m_animation = animation_handle;
		if (animation_handle.is_valid() == false)
		{
			return;
		}

		m_animation_data = &engine_core.get_system<AnimationSystem>().get_animation_data(animation_handle);

		m_sample_data.reset();

		m_sample_data.channels.resize(m_animation_data->channels.size());

		for (size_t channel_index = 0; channel_index < m_animation_data->channels.size(); ++channel_index)
		{
			const AnimationChannel& current_channel = m_animation_data->channels[channel_index];
			AnimationChannelSample& current_channel_sample = m_sample_data.channels[channel_index];

			current_channel_sample.tag = current_channel.tag;
		}
	}

	void AnimationPlayer::set_current_frame(int32_t frame)
	{
		m_current_frame = std::clamp(frame, 0, m_animation_data->frame_count - 1);
		m_current_time = float(m_current_frame);
	}

	void AnimationPlayer::update(float delta_time)
	{
		if (is_looping() == true)
		{
			m_current_time += m_time_scale * delta_time;
			if (m_current_time >= m_animation_data->frame_count)
			{
				m_current_time = std::max(m_current_time - float(m_animation_data->frame_count), 0.0f);
			}
			m_current_frame = int(m_current_time);
		} 
		else if(m_current_time < m_animation_data->frame_count)
		{
			m_current_time += m_time_scale * delta_time;
			m_current_frame = std::clamp(int(m_current_time), 0, m_animation_data->frame_count - 1);
		}

		update_sample_data();
	}

	void AnimationPlayer::reset()
	{
		m_current_frame = 0;
		m_current_time = 0.0f;

		m_sample_data.reset();
	}

	void AnimationPlayer::update_sample_data()
	{
		if (m_animation_data == nullptr)
		{
			return;
		}

		if (m_sample_data.sampled_frame == m_current_frame)
		{
			return;
		}

		for (size_t channel_index = 0; channel_index < m_animation_data->channels.size(); ++channel_index)
		{
			const AnimationChannel& current_channel = m_animation_data->channels[channel_index];
			AnimationChannelSample& current_channel_sample = m_sample_data.channels[channel_index];
			
			if (current_channel.keyframe_range.count == 0)
			{
				// No valid data to return
				current_channel_sample.value = Vadon::Utilities::Variant();
				continue;
			}

			if (m_sample_method == AnimationSampleMethod::LAST_KEYFRAME)
			{
				for (int32_t keyframe_data_index = 0; keyframe_data_index < current_channel.keyframe_range.count; ++keyframe_data_index)
				{
					const AnimationKeyframe& keyframe_data = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + keyframe_data_index];
					if (keyframe_data.frame_index <= m_current_frame)
					{
						current_channel_sample.value = keyframe_data.value;
						break;
					}
				}
				continue;
			}

			if (current_channel.keyframe_range.count == 1)
			{
				current_channel_sample.value = m_animation_data->keyframe_data[current_channel.keyframe_range.offset].value;
				continue;
			}

			// Find the closest keyframe
			int32_t min_keyframe_distance = std::numeric_limits<int32_t>::max();
			int32_t closest_keyframe_index = 0;
			for (int32_t keyframe_data_index = 0; (keyframe_data_index < current_channel.keyframe_range.count) && (min_keyframe_distance > 0); ++keyframe_data_index)
			{
				const AnimationKeyframe& keyframe_data = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + keyframe_data_index];
				if (keyframe_data.frame_index == m_current_frame)
				{
					closest_keyframe_index = keyframe_data_index;
					min_keyframe_distance = 0;
				}
				else
				{
					const int32_t current_distance = std::abs(keyframe_data.frame_index - m_current_frame);
					if (current_distance < min_keyframe_distance)
					{
						closest_keyframe_index = keyframe_data_index;
						min_keyframe_distance = current_distance;
					}
				}
			}

			switch (m_sample_method)
			{
			case AnimationSampleMethod::LINEAR:
			{
				if (min_keyframe_distance > 0)
				{
					const AnimationKeyframe& keyframe_data = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + closest_keyframe_index];

					const bool is_before_closest = m_current_frame < keyframe_data.frame_index;

					const int32_t other_keyframe_index = is_before_closest ? (closest_keyframe_index - 1) : (closest_keyframe_index + 1);

					const AnimationKeyframe& other_keyframe_data = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + other_keyframe_index];

					const int32_t frame_difference = std::abs(other_keyframe_data.frame_index - keyframe_data.frame_index);
					const float keyframe_factor = float(min_keyframe_distance) / float(frame_difference);
					
					current_channel_sample.value = is_before_closest ? 
						lerp_variant_values(keyframe_data.value, other_keyframe_data.value, keyframe_factor)
						: lerp_variant_values(other_keyframe_data.value, keyframe_data.value, 1.0f - keyframe_factor);
				}
				else
				{
					// Exactly on keyframe, use its value
					current_channel_sample.value = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + closest_keyframe_index].value;
				}
			}
				break;
			case AnimationSampleMethod::NEAREST_NEIGHBOR:
				// Just use the value of the closest keyframe
				current_channel_sample.value = m_animation_data->keyframe_data[current_channel.keyframe_range.offset + closest_keyframe_index].value;
				break;
			}
		}

		m_sample_data.sampled_frame = m_current_frame;
	}
}