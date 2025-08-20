#include <Vadon/Private/PCH/Core.hpp>

#include <Vadon/Private/Scene/Animation/AnimationSystem.hpp>

#include <Vadon/Scene/Resource/Registry.hpp>
#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>

#include <Vadon/Utilities/TypeInfo/Reflection/PropertySerialization.hpp>
#include <Vadon/Utilities/TypeInfo/TypeErasure.hpp>

namespace
{
	bool serialize_animation_channel(Vadon::Utilities::Serializer& serializer, Vadon::Scene::AnimationData& animation_data, size_t channel_index)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		Vadon::Scene::AnimationChannel& animation_channel = animation_data.channels[channel_index];

		if (serializer.serialize("tag", animation_channel.tag) != SerializerResult::SUCCESSFUL)
		{
			return false;
		}

		using AnimationChannelDataType = std::underlying_type_t<Vadon::Scene::AnimationChannelType>;
		if (serializer.is_reading() == true)
		{
			AnimationChannelDataType channel_data_type = 0;
			if (serializer.serialize("data_type", channel_data_type) != SerializerResult::SUCCESSFUL)
			{
				return false;
			}

			animation_channel.data_type = Vadon::Utilities::to_enum<Vadon::Scene::AnimationChannelType>(channel_data_type);
		}
		else
		{
			VADON_ASSERT(animation_channel.data_type != Vadon::Scene::AnimationChannelType::INVALID, "Invalid channel type!");
			AnimationChannelDataType channel_data_type = Vadon::Utilities::to_integral(animation_channel.data_type);
			if (serializer.serialize("data_type", channel_data_type) != SerializerResult::SUCCESSFUL)
			{
				return false;
			}
		}

		// Putting indices and values in separate arrays
		// FIXME: might be easier to just put them together?
		if (serializer.open_array("keyframe_indices") != SerializerResult::SUCCESSFUL)
		{
			return false;
		}

		const size_t keyframe_count = (serializer.is_reading() == true) ? serializer.get_array_size() : animation_channel.keyframe_range.count;
		if (serializer.is_reading() == true)
		{
			animation_channel.keyframe_range.offset = static_cast<int32_t>(animation_data.keyframe_data.size());
			animation_channel.keyframe_range.count = static_cast<int32_t>(keyframe_count);

			animation_data.keyframe_data.insert(animation_data.keyframe_data.end(), keyframe_count, Vadon::Scene::AnimationKeyframe{});
		}

		for (size_t keyframe_index = 0; keyframe_index < keyframe_count; ++keyframe_index)
		{
			Vadon::Scene::AnimationKeyframe& keyframe_data = animation_data.keyframe_data[animation_channel.keyframe_range.offset + keyframe_index];
			if (serializer.serialize(keyframe_index, keyframe_data.frame_index) != SerializerResult::SUCCESSFUL)
			{
				return false;
			}
		}

		if (serializer.close_array() != SerializerResult::SUCCESSFUL)
		{
			return false;
		}

		if (serializer.open_array("keyframe_values") != SerializerResult::SUCCESSFUL)
		{
			return false;
		}

		if (serializer.is_reading() == true)
		{
			VADON_ASSERT(animation_channel.keyframe_range.count == serializer.get_array_size(), "Mismatch in channel data!");
		}

		for (size_t keyframe_index = 0; keyframe_index < keyframe_count; ++keyframe_index)
		{
			Vadon::Scene::AnimationKeyframe& keyframe_data = animation_data.keyframe_data[animation_channel.keyframe_range.offset + keyframe_index];
			if (Vadon::Utilities::process_trivial_property(serializer, keyframe_index, keyframe_data.value, Vadon::Scene::AnimationChannel::get_data_type_id(animation_channel.data_type)) != SerializerResult::SUCCESSFUL)
			{
				return false;
			}
		}

		if (serializer.close_array() != SerializerResult::SUCCESSFUL)
		{
			return false;
		}

		return true;
	}

	bool serialize_animation_data(Vadon::Scene::ResourceSystem& resource_system, Vadon::Utilities::Serializer& serializer, Vadon::Scene::Resource& resource)
	{
		using SerializerResult = Vadon::Utilities::Serializer::Result;
		constexpr const char* c_error_message = "Animation system: unable to serialize animation data!\n";

		Vadon::Scene::Animation& animation = static_cast<Vadon::Scene::Animation&>(resource);
		Vadon::Scene::AnimationData& animation_data = animation.data;

		if (serializer.serialize("frame_count", animation_data.frame_count) != SerializerResult::SUCCESSFUL)
		{
			resource_system.log_error(c_error_message);
			return false;
		}

		if (serializer.open_array("channels") != SerializerResult::SUCCESSFUL)
		{
			resource_system.log_error(c_error_message);
			return false;
		}

		const size_t channel_count = (serializer.is_reading() == true) ? serializer.get_array_size() : animation_data.channels.size();
		if (serializer.is_reading() == true)
		{
			animation_data.channels.resize(channel_count);
			animation_data.keyframe_data.clear();
		}

		for (size_t channel_index = 0; channel_index < channel_count; ++channel_index)
		{
			if (serializer.open_object(channel_index) != SerializerResult::SUCCESSFUL)
			{
				resource_system.log_error(c_error_message);
				return false;
			}

			if (serialize_animation_channel(serializer, animation_data, channel_index) == false)
			{
				resource_system.log_error(c_error_message);
				return false;
			}

			if (serializer.close_object() != SerializerResult::SUCCESSFUL)
			{
				resource_system.log_error(c_error_message);
				return false;
			}
		}

		if (serializer.close_array() != SerializerResult::SUCCESSFUL)
		{
			resource_system.log_error(c_error_message);
			return false;
		}

		return true;
	}
}

namespace Vadon::Scene
{
	void Animation::register_type_info()
	{
		Vadon::Scene::ResourceRegistry::register_resource_type<Animation, Resource>();

		Vadon::Scene::ResourceRegistry::register_resource_serializer<Animation>(&serialize_animation_data);
	}
}
namespace Vadon::Private::Scene
{
	AnimationHandle AnimationSystem::create_animation()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		AnimationHandle new_animation_handle = AnimationHandle::from_resource_handle(resource_system.create_resource<Vadon::Scene::Animation>());

		if (new_animation_handle.is_valid() == false)
		{
			log_error("Animation system: failed to create scene!\n");
			return new_animation_handle;
		}

		return new_animation_handle;
	}

	AnimationHandle AnimationSystem::find_animation(AnimationID anim_id) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const Vadon::Scene::ResourceHandle animation_handle = resource_system.find_resource(anim_id);
		if (animation_handle.is_valid() == false)
		{
			return AnimationHandle();
		}

		if (resource_system.get_resource_info(animation_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Animation>())
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
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const Vadon::Scene::ResourceHandle animation_resource_handle = resource_system.load_resource(anim_id);
		if (resource_system.get_resource_info(animation_resource_handle).type_id != Vadon::Utilities::TypeRegistry::get_type_id<Vadon::Scene::Animation>())
		{
			// TODO: log resource ID!
			// TODO2: remove loaded resource?
			log_error("Animation system: resource ID does not correspond to animation resource!\n");
			return AnimationHandle();
		}
		return AnimationHandle::from_resource_handle(animation_resource_handle);
	}

	const AnimationData& AnimationSystem::get_animation_data(AnimationHandle animation_handle) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		Vadon::Scene::Animation* animation = resource_system.get_resource<Vadon::Scene::Animation>(animation_handle);

		return animation->data;
	}

	void AnimationSystem::set_animation_data(AnimationHandle animation_handle, const AnimationData& data)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
		Vadon::Scene::Animation* animation = resource_system.get_resource<Vadon::Scene::Animation>(animation_handle);

		animation->data = data;
	}

	AnimationSystem::AnimationSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Scene::AnimationSystem(core)
	{

	}

	bool AnimationSystem::initialize()
	{
		log_message("Initializing Animation System\n");
		Vadon::Scene::Animation::register_type_info();
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