#ifndef VADON_PRIVATE_MODEL_ANIMATION_ANIMATIONSYSTEM_HPP
#define VADON_PRIVATE_MODEL_ANIMATION_ANIMATIONSYSTEM_HPP
#include <Vadon/Model/Animation/AnimationSystem.hpp>
#include <Vadon/Private/Model/Animation/Animation.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace Vadon::Private::Model
{
	class AnimationSystem final : public Vadon::Model::AnimationSystem
	{
	public:
		AnimationHandle create_animation() override;
		AnimationHandle find_animation(AnimationID anim_id) const override;

		AnimationHandle load_animation(AnimationID anim_id) override;

		const AnimationData& get_animation_data(AnimationHandle animation_handle) const override;
		void set_animation_data(AnimationHandle animation_handle, const AnimationData& data) override;
	private:
		AnimationSystem(Vadon::Core::EngineCoreInterface& core);

		static void register_types();
		static void register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);

		bool initialize();
		void shutdown();

		friend class SceneSystem;
	};
}
#endif