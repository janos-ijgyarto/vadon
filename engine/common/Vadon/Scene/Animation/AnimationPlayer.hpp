#ifndef VADON_SCENE_ANIMATION_ANIMATIONPLAYER_HPP
#define VADON_SCENE_ANIMATION_ANIMATIONPLAYER_HPP
#include <Vadon/Scene/Animation/Animation.hpp>
#include <span>
namespace Vadon::Core
{
	class EngineCoreInterface;
}

namespace Vadon::Scene
{
	struct AnimationChannelSample
	{
		std::string_view tag;
		Vadon::Utilities::Variant value;
	};

	struct AnimationSample
	{
		std::span<const AnimationChannelSample> channels;
	};

	enum class AnimationSampleMethod
	{
		LINEAR,
		NEAREST_NEIGHBOR,
		LAST_KEYFRAME // Similar to NN but it is set to whichever keyframe was last passed
	};

	class AnimationPlayer
	{
	public:
		AnimationPlayer()
			: m_animation_data(nullptr)
			, m_current_frame(0)
			, m_time_scale(1.0f)
			, m_looping(false)
			, m_sample_method(AnimationSampleMethod::LINEAR)
			, m_current_time(0.0f)
		{
		}

		AnimationHandle get_animation() const { return m_animation; }
		void set_animation(Vadon::Core::EngineCoreInterface& engine_core, AnimationHandle animation_handle);

		int32_t get_current_frame() const { return m_current_frame; }
		void set_current_frame(int32_t frame);

		float get_time_scale() const { return m_time_scale; }
		void set_time_scale(float time_scale) { m_time_scale = time_scale; }

		bool is_looping() const { return m_looping; }
		void set_looping(bool looping) { m_looping = looping; }

		AnimationSampleMethod get_sample_method() const { return m_sample_method; }
		void set_sample_method(AnimationSampleMethod sample_method) { m_sample_method = sample_method; }

		void update(float delta_time);
		void reset();

		AnimationSample get_sample() const { return AnimationSample{ .channels = m_sample_data.channels }; }
	private:
		struct SampleData
		{
			std::vector<AnimationChannelSample> channels;
			int32_t sampled_frame = -1;

			void reset() { sampled_frame = -1; }
		};

		void update_sample_data();

		AnimationHandle m_animation;
		const AnimationData* m_animation_data;

		int32_t m_current_frame;
		float m_time_scale;
		bool m_looping;
		AnimationSampleMethod m_sample_method;

		float m_current_time;
		SampleData m_sample_data;
	};
}
#endif