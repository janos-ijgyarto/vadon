#ifndef VADON_SCENE_ANIMATION_ANIMATIONSYSTEM_HPP
#define VADON_SCENE_ANIMATION_ANIMATIONSYSTEM_HPP
#include <Vadon/Scene/Module.hpp>
#include <Vadon/Scene/Animation/Animation.hpp>
namespace Vadon::Scene
{
	class AnimationSystem : public SceneSystemBase<AnimationSystem>
	{
	public:
		// NOTE: these are convenience functions, equivalent to calling ResourceSystem directly
		virtual AnimationHandle create_animation() = 0;

		virtual AnimationHandle find_animation(AnimationID anim_id) const = 0;

		virtual AnimationHandle load_animation(AnimationID anim_id) = 0;

		virtual const AnimationData& get_animation_data(AnimationHandle animation_handle) const = 0;
		virtual void set_animation_data(AnimationHandle animation_handle, const AnimationData& data) = 0;
	protected:
		AnimationSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif