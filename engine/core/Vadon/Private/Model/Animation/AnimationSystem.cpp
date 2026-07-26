#include <Vadon/Private/Model/Animation/AnimationSystem.hpp>

#include <Vadon/Model/Resource/Registry.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

#define VADON_REGISTER_ANIMATION_CHANNEL_TYPE(_type_name, _member_name) TypeRegistry::register_type<_type_name, AnimationChannel>();\
TypeRegistry::add_property<_type_name>(VADON_GET_MEMBER_UUID(AnimationChannel, _member_name), Vadon::Utilities::MemberVariableBind<&_type_name::key_data>().bind_member_getter().bind_member_setter())

#define VADON_REGISTER_ANIMATION_CHANNEL_METADATA(_registry, _type_name, _member_name) {\
Vadon::Utilities::TypeMetadata animation_channel_metadata(_registry, VADON_GET_TYPE_UUID(_type_name)); \
animation_channel_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, #_type_name)\
.add_property(VADON_GET_MEMBER_UUID(_type_name, _member_name))\
.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Data")\
.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN));\
}
namespace Vadon::Private::Model
{
	AnimationHandle AnimationSystem::create_animation()
	{
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		AnimationHandle new_animation_handle = AnimationHandle::from_resource_handle(resource_system.create_resource<Vadon::Model::Animation>());

		if (new_animation_handle.is_valid() == false)
		{
			log_error("Animation system: failed to create scene!\n");
			return new_animation_handle;
		}

		return new_animation_handle;
	}

	AnimationHandle AnimationSystem::find_animation(AnimationID anim_id) const
	{
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		const Vadon::Model::ResourceHandle animation_handle = resource_system.find_resource(anim_id);
		if (animation_handle.is_valid() == false)
		{
			return AnimationHandle();
		}

		if (resource_system.get_resource_info(animation_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Model::Animation>())
		{
			// TODO: log resource ID!
			log_error("Animation system: resource ID does not correspond to animation resource!\n");
			return AnimationHandle();
		}

		return AnimationHandle::from_resource_handle(animation_handle);
	}

	AnimationHandle AnimationSystem::load_animation(AnimationID anim_id)
	{
		VADON_ASSERT(anim_id.is_valid() == true, "Cannot load invalid ID!");
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		const Vadon::Model::ResourceHandle animation_resource_handle = resource_system.load_resource(anim_id);
		if (resource_system.get_resource_info(animation_resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Model::Animation>())
		{
			// TODO: log resource ID!
			// TODO2: remove loaded resource?
			log_error("Animation system: resource ID does not correspond to animation resource!\n");
			return AnimationHandle();
		}
		return AnimationHandle::from_resource_handle(animation_resource_handle);
	}

	const Animation* AnimationSystem::get_animation_data(AnimationHandle anim_handle) const
	{
		Vadon::Model::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Model::ResourceSystem>();
		return resource_system.get_resource(anim_handle);
	}

	AnimationSystem::AnimationSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Model::AnimationSystem(core)
	{

	}

	void AnimationSystem::register_types()
	{
		using TypeRegistry = Vadon::Utilities::TypeRegistry;

		TypeRegistry::register_type<AnimationChannel>();

		TypeRegistry::add_property<AnimationChannel>(VADON_GET_MEMBER_UUID(AnimationChannel, id), Vadon::Utilities::MemberVariableBind<&AnimationChannel::id>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationChannel>(VADON_GET_MEMBER_UUID(AnimationChannel, key_times), Vadon::Utilities::MemberVariableBind<&AnimationChannel::key_times>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationChannel>(VADON_GET_MEMBER_UUID(AnimationChannel, tag), Vadon::Utilities::MemberVariableBind<&AnimationChannel::tag>().bind_member_getter().bind_member_setter());

		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationIntChannel, int_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationUintChannel, uint_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationFloatChannel, float_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationVector2Channel, vector2_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationVector2iChannel, vector2i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationVector3Channel, vector3_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationVector3iChannel, vector3i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationVector4Channel, vector4_data);
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationColorRGBAChannel, colorrgba_data);

		Vadon::Model::ResourceRegistry::register_resource_type<Animation, Vadon::Model::Resource>();
		TypeRegistry::add_property<Animation>(VADON_GET_MEMBER_UUID(Animation, channels), Vadon::Utilities::MemberVariableBind<&Animation::channels>().bind_member_getter().bind_setter_function<&Animation::set_channels>());
	}

	void AnimationSystem::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		{
			Vadon::Utilities::TypeMetadata animation_metadata(metadata_registry, VADON_GET_TYPE_UUID(Animation));
			animation_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::Animation")
				.add_property(VADON_GET_MEMBER_UUID(Animation, channels))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Channels")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN));
			
			Vadon::Utilities::TypeMetadata animation_channel_metadata(metadata_registry, VADON_GET_TYPE_UUID(AnimationChannel));
			animation_channel_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::AnimationChannel")
				.add_property(VADON_GET_MEMBER_UUID(AnimationChannel, id))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "ID")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(AnimationChannel, tag))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Tag")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(AnimationChannel, key_times))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Key times")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property();
		}

		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationIntChannel, int_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationUintChannel, uint_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationFloatChannel, float_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationVector2Channel, vector2_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationVector2iChannel, vector2i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationVector3Channel, vector3_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationVector3iChannel, vector3i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationVector4Channel, vector4_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, Vadon::Model::AnimationColorRGBAChannel, colorrgba_data);
	}

	bool AnimationSystem::initialize()
	{
		log_message("Initializing Animation System\n");
		// TODO: anything?
		log_message("Animation System initialized!\n");
		return true;
	}

	void AnimationSystem::shutdown()
	{
		log_message("Shutting down Animation System\n");
		// TODO: anything?
		log_message("Animation System shut down!\n");
	}
}