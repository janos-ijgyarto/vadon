#include <Vadon/Private/Model/Animation/AnimationSystem.hpp>

#include <Vadon/Private/Model/Animation/EntityAnimation.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Model/Resource/Registry.hpp>
#include <Vadon/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/MemberBind.hpp>

#include <Vadon/Utilities/TypeInfo/Metadata.hpp>

VADON_DEFINE_ENUM_TYPE_ERASURE(Vadon::Model::AnimationChannelSampleMethod);
VADON_DEFINE_ENUM_VARIANT_TYPE_TRAIT(Vadon::Model::AnimationChannelSampleMethod);

#define VADON_REGISTER_ANIMATION_CHANNEL_TYPE(_type_name, _member_name) TypeRegistry::register_type<_type_name, AnimationChannel>();\
TypeRegistry::register_type_factory<_type_name>();\
TypeRegistry::add_property<_type_name>(VADON_GET_MEMBER_UUID(AnimationChannel, _member_name), Vadon::Utilities::MemberVariableBind<&_type_name::key_data>().bind_member_getter().bind_member_setter())

#define VADON_REGISTER_ANIMATION_CHANNEL_METADATA(_registry, _type_name, _member_name) {\
Vadon::Utilities::TypeMetadata<_type_name> animation_channel_metadata(_registry); \
animation_channel_metadata.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, #_type_name)\
.add_property(VADON_GET_MEMBER_UUID(_type_name, _member_name))\
.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Data")\
.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN));\
}

VADON_DEFINE_OBJECT_VARIANT_TYPE_TRAIT(Vadon::Model::EntityAnimationEntry);

namespace
{
	constexpr ::Vadon::Foundation::UUID get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod method)
	{
		return Vadon::Utilities::UUIDLiteral(::Vadon::Foundation::AnimationChannelSchema::get_sample_method_uuid_string(method).string).result;
	}
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

		TypeRegistry::register_enum_type<AnimationChannelSampleMethod>();
		TypeRegistry::add_enum_value<AnimationChannelSampleMethod>(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LINEAR), static_cast<size_t>(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LINEAR));
		TypeRegistry::add_enum_value<AnimationChannelSampleMethod>(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::NEAREST_NEIGHBOR), static_cast<size_t>(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::NEAREST_NEIGHBOR));
		TypeRegistry::add_enum_value<AnimationChannelSampleMethod>(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LAST_KEY), static_cast<size_t>(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LAST_KEY));

		TypeRegistry::add_property<AnimationChannel>(VADON_GET_MEMBER_UUID(AnimationChannel, id), Vadon::Utilities::MemberVariableBind<&AnimationChannel::id>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<AnimationChannel>(VADON_GET_MEMBER_UUID(AnimationChannel, sample_method), Vadon::Utilities::MemberVariableBind<&AnimationChannel::sample_method>().bind_member_getter().bind_member_setter());
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
		VADON_REGISTER_ANIMATION_CHANNEL_TYPE(AnimationUUIDChannel, uuid_data);

		Vadon::Model::ResourceRegistry::register_resource_type<Animation, Vadon::Model::Resource>();
		TypeRegistry::add_property<Animation>(VADON_GET_MEMBER_UUID(Animation, channels), Vadon::Utilities::MemberVariableBind<&Animation::channels>().bind_member_getter().bind_setter_function<&Animation::set_channels>());
		TypeRegistry::add_property<Animation>(VADON_GET_MEMBER_UUID(Animation, time_scale), Vadon::Utilities::MemberVariableBind<&Animation::time_scale>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<Animation>(VADON_GET_MEMBER_UUID(Animation, looping), Vadon::Utilities::MemberVariableBind<&Animation::looping>().bind_member_getter().bind_member_setter());

		Vadon::Model::ResourceRegistry::register_resource_type<AnimationLibrary, Vadon::Model::Resource>();
		TypeRegistry::add_property<AnimationLibrary>(VADON_GET_MEMBER_UUID(AnimationLibrary, animations), Vadon::Utilities::MemberVariableBind<&AnimationLibrary::animations>().bind_member_getter().bind_member_setter());

		TypeRegistry::register_type<EntityAnimationEntry>();

		TypeRegistry::add_property<EntityAnimationEntry>(VADON_GET_MEMBER_UUID(EntityAnimationEntry, animation), Vadon::Utilities::MemberVariableBind<&EntityAnimationEntry::animation>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EntityAnimationEntry>(VADON_GET_MEMBER_UUID(EntityAnimationEntry, channel), Vadon::Utilities::MemberVariableBind<&EntityAnimationEntry::channel>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EntityAnimationEntry>(VADON_GET_MEMBER_UUID(EntityAnimationEntry, entity), Vadon::Utilities::MemberVariableBind<&EntityAnimationEntry::entity>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EntityAnimationEntry>(VADON_GET_MEMBER_UUID(EntityAnimationEntry, component), Vadon::Utilities::MemberVariableBind<&EntityAnimationEntry::component>().bind_member_getter().bind_member_setter());
		TypeRegistry::add_property<EntityAnimationEntry>(VADON_GET_MEMBER_UUID(EntityAnimationEntry, property), Vadon::Utilities::MemberVariableBind<&EntityAnimationEntry::property>().bind_member_getter().bind_member_setter());

		Vadon::Model::ResourceRegistry::register_resource_type<EntityAnimation, AnimationLibrary>();

		TypeRegistry::add_property<EntityAnimation>(VADON_GET_MEMBER_UUID(EntityAnimation, entries), Vadon::Utilities::MemberVariableBind<&EntityAnimation::entries>().bind_member_getter().bind_member_setter());

		Vadon::ECS::ComponentRegistry::register_component_type<AnimationComponent>();

		TypeRegistry::add_property<AnimationComponent>(VADON_GET_MEMBER_UUID(AnimationComponent, animation), Vadon::Utilities::MemberVariableBind<&AnimationComponent::animation>().bind_member_getter().bind_member_setter());
	}

	void AnimationSystem::register_type_metadata(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry)
	{
		{
			Vadon::Utilities::TypeMetadata<Animation>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::Animation")
				.add_property(VADON_GET_MEMBER_UUID(Animation, channels))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Channels")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(Animation, time_scale))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Time Scale")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(Animation, looping))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Looping")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN));
			
			Vadon::Utilities::EnumMetadata<AnimationChannelSampleMethod>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::AnimationChannelSampleMethod")
				.add_value(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LINEAR))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Linear")
					.commit_value()
				.add_value(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::NEAREST_NEIGHBOR))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Nearest Neighbor")
					.commit_value()
				.add_value(get_animation_channel_sample_method_uuid(::Vadon::Foundation::AnimationChannelSchema::SampleMethod::LAST_KEY))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Last Key")
					.commit_value();

			Vadon::Utilities::TypeMetadata<AnimationChannel>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::AnimationChannel")
				.add_property(VADON_GET_MEMBER_UUID(AnimationChannel, id))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "ID")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(AnimationChannel, sample_method))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Sample method")
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

			Vadon::Utilities::TypeMetadata<AnimationLibrary>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::AnimationLibrary")
				.add_property(VADON_GET_MEMBER_UUID(AnimationLibrary, animations))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Animations")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property();

			Vadon::Utilities::TypeMetadata<EntityAnimation>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::EntityAnimation")
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimation, entries))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Entries")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property();

			Vadon::Utilities::TypeMetadata<EntityAnimationEntry>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::EntityAnimationEntry")
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimationEntry, animation))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Animation")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimationEntry, channel))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Channel")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimationEntry, entity))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Entity")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimationEntry, component))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Component")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property()
				.add_property(VADON_GET_MEMBER_UUID(EntityAnimationEntry, property))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Property")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property();

			Vadon::Utilities::TypeMetadata<AnimationComponent>(metadata_registry)
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME, "Vadon::Model::AnimationComponent")
				.add_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT, "Vadon/Model")
				.add_property(VADON_GET_MEMBER_UUID(AnimationComponent, animation))
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME, "Animation")
					.add_metadata(::Vadon::Foundation::CommonPropertyMetadata::FLAGS, ::Vadon::Foundation::CommonPropertyMetadata::flag_string(::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN))
					.commit_property();
		}

		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationIntChannel, int_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationUintChannel, uint_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationFloatChannel, float_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationVector2Channel, vector2_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationVector2iChannel, vector2i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationVector3Channel, vector3_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationVector3iChannel, vector3i_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationVector4Channel, vector4_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationColorRGBAChannel, colorrgba_data);
		VADON_REGISTER_ANIMATION_CHANNEL_METADATA(metadata_registry, AnimationUUIDChannel, uuid_data);
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