#ifndef VADON_PRIVATE_SCENE_ANIMATION_ANIMATIONSYSTEM_HPP
#define VADON_PRIVATE_SCENE_ANIMATION_ANIMATIONSYSTEM_HPP
#include <Vadon/Scene/Animation/AnimationSystem.hpp>
#include <Vadon/Private/Scene/Animation/Animation.hpp>
namespace Vadon::Private::Scene
{
	class AnimationSystem final : public Vadon::Scene::AnimationSystem
	{
	public:
		AnimationHandle create_animation() override;
		AnimationHandle find_animation(AnimationID anim_id) const override;

		AnimationHandle load_animation(AnimationID anim_id) override;

		const AnimationData& get_animation_data(AnimationHandle animation_handle) const override;
		void set_animation_data(AnimationHandle animation_handle, const AnimationData& data) override;
	private:
		AnimationSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		friend class SceneSystem;
	};
}
#endif