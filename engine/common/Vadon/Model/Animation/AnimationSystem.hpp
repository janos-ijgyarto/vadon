#ifndef VADON_MODEL_ANIMATION_ANIMATIONSYSTEM_HPP
#define VADON_MODEL_ANIMATION_ANIMATIONSYSTEM_HPP
#include <Vadon/Model/Module.hpp>
#include <Vadon/Model/Animation/Animation.hpp>
namespace Vadon::Model
{
	class AnimationSystem : public ModelSystemBase<AnimationSystem>
	{
	public:
		// NOTE: these are convenience functions, equivalent to calling ResourceSystem directly
		virtual AnimationHandle create_animation() = 0;

		virtual AnimationHandle find_animation(AnimationID anim_id) const = 0;

		virtual AnimationHandle load_animation(AnimationID anim_id) = 0;

		virtual const Animation* get_animation_data(AnimationHandle anim_handle) const = 0;
	protected:
		AnimationSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif