#include <Vadon/Model/Animation/AnimationPlayer.hpp>

#include <Vadon/Model/Animation/AnimationSystem.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>
#include <Vadon/Utilities/TypeInfo/TypeList/VariantTypeList.hpp>

#include <algorithm>

namespace
{
	Vadon::Utilities::Variant lerp_variant_values(const Vadon::Utilities::Variant& value_a, const Vadon::Utilities::Variant& value_b, float factor)
	{
		// FIXME: use std::holds_alternative?
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

namespace Vadon::Model
{
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

		const Animation* animation_data = engine_core.get_system<AnimationSystem>().get_animation_data(animation_handle);

		m_sample_data.reset();

		m_sample_data.channels.resize(animation_data->channels.size());

		for (size_t channel_index = 0; channel_index < animation_data->channels.size(); ++channel_index)
		{
			const AnimationChannel& current_channel = *animation_data->channels[channel_index];
			AnimationChannelSample& current_channel_sample = m_sample_data.channels[channel_index];

			current_channel_sample.id = current_channel.id;
			current_channel_sample.tag = current_channel.tag;
		}
	}

	void AnimationPlayer::set_current_time(float time)
	{
		m_current_time = std::clamp(time, 0.0f, 1.0f);
	}

	void AnimationPlayer::update(Vadon::Core::EngineCoreInterface& engine_core, float delta_time)
	{
		if (is_looping() == true)
		{
			m_current_time += m_time_scale * delta_time;
			if (m_current_time >= 1.0f)
			{
				m_current_time -= 1.0f;
			}
		} 
		else if(m_current_time < 1.0f)
		{
			m_current_time = std::min(m_current_time + (m_time_scale * delta_time), 1.0f);
		}

		update_sample_data(engine_core);
	}

	void AnimationPlayer::reset()
	{
		m_current_time = 0.0f;

		m_sample_data.reset();
	}

	void AnimationPlayer::update_sample_data(Vadon::Core::EngineCoreInterface& engine_core)
	{
		if (m_animation.is_valid() == false)
		{
			return;
		}

		if (m_sample_data.sampled_time == m_current_time)
		{
			return;
		}

		const Animation* animation_data = engine_core.get_system<AnimationSystem>().get_animation_data(m_animation);

		for (size_t channel_index = 0; channel_index < animation_data->channels.size(); ++channel_index)
		{
			const AnimationChannel& current_channel = *animation_data->channels[channel_index];
			AnimationChannelSample& current_channel_sample = m_sample_data.channels[channel_index];
			
			if (current_channel.get_key_count() == 0)
			{
				// No valid data to return
				current_channel_sample.value = Vadon::Utilities::Variant();
				continue;
			}

			if (m_sample_method == AnimationSampleMethod::LAST_KEY)
			{
				for (size_t key_index = 0; key_index < current_channel.get_key_count(); ++key_index)
				{
					const AnimationKey key_data = current_channel.get_key(key_index);
					if (key_data.time <= m_current_time)
					{
						current_channel_sample.value = key_data.value;
						break;
					}
				}
				continue;
			}

			if (current_channel.get_key_count() == 1)
			{
				current_channel_sample.value = current_channel.get_key_data(0);
				continue;
			}

			// Find the closest key
			float min_key_distance = 1.0f;
			size_t closest_key_index = 0;
			for (size_t key_data_index = 0; (key_data_index < current_channel.get_key_count()) && (min_key_distance > 0); ++key_data_index)
			{
				const AnimationKey key_data = current_channel.get_key(key_data_index);
				if (key_data.time == m_current_time)
				{
					closest_key_index = key_data_index;
					min_key_distance = 0;
				}
				else
				{
					const float current_distance = std::abs(key_data.time - m_current_time);
					if (current_distance < min_key_distance)
					{
						closest_key_index = key_data_index;
						min_key_distance = current_distance;
					}
				}
			}

			switch (m_sample_method)
			{
			case AnimationSampleMethod::LINEAR:
			{
				if (min_key_distance > 0)
				{
					const AnimationKey& key_data = current_channel.get_key(closest_key_index);

					const bool is_before_closest = m_current_time < key_data.time;

					size_t other_key_index = 0;
					if (is_before_closest)
					{
						if (closest_key_index > 0)
						{
							other_key_index = closest_key_index - 1;
						}
						else
						{
							// Just use the closest value
							// TODO: "wrap around" and lerp toward last key?
							current_channel_sample.value = current_channel.get_key_data(closest_key_index);
							continue;
						}
					}
					else
					{
						other_key_index = closest_key_index + 1;
						if (other_key_index >= current_channel.get_key_count())
						{
							// Just use the closest value
							// TODO: "wrap around" and lerp toward first key?
							current_channel_sample.value = current_channel.get_key_data(closest_key_index);
							continue;
						}
					}

					const AnimationKey& other_key_data = current_channel.get_key(other_key_index);

					const float time_difference = std::abs(other_key_data.time - key_data.time);
					const float key_factor = float(min_key_distance) / float(time_difference);
					
					current_channel_sample.value = is_before_closest ? 
						lerp_variant_values(key_data.value, other_key_data.value, key_factor)
						: lerp_variant_values(other_key_data.value, key_data.value, 1.0f - key_factor);
				}
				else
				{
					// Exactly on key, use its value
					current_channel_sample.value = current_channel.get_key_data(closest_key_index);
				}
			}
				break;
			case AnimationSampleMethod::NEAREST_NEIGHBOR:
				// Just use the value of the closest key
				current_channel_sample.value = current_channel.get_key_data(closest_key_index);
				break;
			}
		}

		m_sample_data.sampled_time = m_current_time;
	}
}