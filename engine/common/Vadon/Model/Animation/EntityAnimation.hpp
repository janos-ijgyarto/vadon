#ifndef VADON_MODEL_ANIMATION_ENTITYANIMATION_HPP
#define VADON_MODEL_ANIMATION_ENTITYANIMATION_HPP
#include <Vadon/Model/Animation/AnimationPlayer.hpp>
namespace Vadon::Model
{
	struct EntityAnimationEntry
	{
		VADON_DECLARE_MEMBER_UUID(animation, ::Vadon::Foundation::EntityAnimationEntrySchema::c_animation_property.id.string);
		VADON_DECLARE_MEMBER_UUID(channel, ::Vadon::Foundation::EntityAnimationEntrySchema::c_channel_property.id.string);
		VADON_DECLARE_MEMBER_UUID(entity, ::Vadon::Foundation::EntityAnimationEntrySchema::c_entity_property.id.string);
		VADON_DECLARE_MEMBER_UUID(component, ::Vadon::Foundation::EntityAnimationEntrySchema::c_component_property.id.string);
		VADON_DECLARE_MEMBER_UUID(property, ::Vadon::Foundation::EntityAnimationEntrySchema::c_property_id_property.id.string);

		AnimationID animation;
		::Vadon::Foundation::UUID channel;
		::Vadon::Foundation::UUID entity;
		::Vadon::Foundation::UUID component;
		::Vadon::Foundation::UUID property;
	};

	struct EntityAnimation : public AnimationLibrary
	{
		VADON_DECLARE_MEMBER_UUID(entries, ::Vadon::Foundation::EntityAnimationSchema::c_entries_property.id.string);

		std::vector<EntityAnimationEntry> entries;
	};

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(EntityAnimation, EntityAnimationID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(EntityAnimation, EntityAnimationHandle);

	struct AnimationComponent
	{
		VADON_DECLARE_MEMBER_UUID(animation, ::Vadon::Foundation::AnimationComponentSchema::c_animation_property.id.string);

		EntityAnimationID animation;
		EntityAnimationHandle anim_handle;

		AnimationPlayer player;
	};
}
VADON_REGISTER_TYPE_UUID(Vadon::Model::EntityAnimationEntry, ::Vadon::Foundation::EntityAnimationEntrySchema::c_type_uuid.string);
VADON_DEFINE_OBJECT_TYPE_ERASURE(Vadon::Model::EntityAnimationEntry);

VADON_REGISTER_TYPE_UUID(Vadon::Model::EntityAnimation, ::Vadon::Foundation::EntityAnimationSchema::c_type_uuid.string);

VADON_REGISTER_TYPE_UUID(Vadon::Model::AnimationComponent, ::Vadon::Foundation::AnimationComponentSchema::c_type_uuid.string);
#endif