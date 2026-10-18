#ifndef VADON_MODEL_ANIMATION_ANIMATIONPLAYER_HPP
#define VADON_MODEL_ANIMATION_ANIMATIONPLAYER_HPP
#include <Vadon/Model/Animation/Animation.hpp>
#include <span>
namespace Vadon::Core
{
	class EngineCoreInterface;
}

namespace Vadon::Model
{
	struct AnimationChannelSample
	{
		::Vadon::Foundation::UUID id;
		std::string tag;
		Vadon::Utilities::Variant value;
	};

	struct AnimationSample
	{
		std::span<const AnimationChannelSample> channels;
	};

	class AnimationPlayer
	{
	public:
		AnimationPlayer()
			: m_current_time(0.0f)
			, m_time_scale(1.0f)
			, m_looping(false)
		{
		}

		AnimationHandle get_animation() const { return m_animation; }
		VADONCOMMON_API void set_animation(Vadon::Core::EngineCoreInterface& engine_core, AnimationHandle animation_handle);

		VADONCOMMON_API void set_current_time(float time);

		float get_time_scale() const { return m_time_scale; }
		void set_time_scale(float time_scale) { m_time_scale = time_scale; }

		bool is_looping() const { return m_looping; }
		void set_looping(bool looping) { m_looping = looping; }

		VADONCOMMON_API void update(Vadon::Core::EngineCoreInterface& engine_core, float delta_time);
		VADONCOMMON_API void reset();

		AnimationSample get_sample() const { return AnimationSample{ .channels = m_sample_data.channels }; }
	private:
		struct SampleData
		{
			std::vector<AnimationChannelSample> channels;
			float sampled_time = -1;

			void reset() { sampled_time = -1; }
		};

		void update_sample_data(Vadon::Core::EngineCoreInterface& engine_core);

		AnimationHandle m_animation;

		float m_current_time;
		float m_time_scale;
		bool m_looping;

		SampleData m_sample_data;
	};
}
#endif