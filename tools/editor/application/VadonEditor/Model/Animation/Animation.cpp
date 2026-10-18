#include <VadonEditor/Model/Animation/Animation.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Animation/Animation.hpp>

#include <QVector2D>

namespace
{
	QVariant get_animation_key_default_value(VadonEditor::Model::AnimationChannelType channel_type)
	{
		switch (channel_type)
		{
		case VadonEditor::Model::AnimationChannelType::INT:
			return QVariant(int(0));
		case VadonEditor::Model::AnimationChannelType::UINT:
			return QVariant(uint(0));
		case VadonEditor::Model::AnimationChannelType::FLOAT:
			return QVariant(0.0f);
		case VadonEditor::Model::AnimationChannelType::VECTOR2:
			return QVariant(QVector2D());
		case VadonEditor::Model::AnimationChannelType::VECTOR2I:
			return QVariant(QPoint());
		case VadonEditor::Model::AnimationChannelType::VECTOR3:
			return QVariant(QVector3D());
		case VadonEditor::Model::AnimationChannelType::VECTOR3I:
			return QVariant(QLine(0, 0, 0, 0));
		case VadonEditor::Model::AnimationChannelType::VECTOR4:
			return QVariant(QVector4D());
		case VadonEditor::Model::AnimationChannelType::COLORRGBA:
			return QVariant(QColor());
		case VadonEditor::Model::AnimationChannelType::UUID:
			return QVariant(QUuid());
		default:
			return QVariant();
		}
	}

	QUuid get_animation_channel_type_uuid(VadonEditor::Model::AnimationChannelType channel_type)
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(channel_type));
	}

	QUuid get_animation_channels_property_uuid()
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationSchema::c_channels_property.id);
	}

	VadonEditor::Model::AnimationChannelType get_animation_channel_type_from_uuid(const QUuid& channel_type_uuid)
	{
		for (::Vadon::Foundation::uint32 index = 0; index < static_cast<::Vadon::Foundation::uint32>(VadonEditor::Model::AnimationChannelType::TYPE_COUNT); ++index)
		{
			const VadonEditor::Model::AnimationChannelType current_type = static_cast<VadonEditor::Model::AnimationChannelType>(index);

			const QUuid current_type_uuid = get_animation_channel_type_uuid(current_type);
			if (current_type_uuid == channel_type_uuid)
			{
				return current_type;
			}
		}

		return VadonEditor::Model::AnimationChannelType::TYPE_COUNT;
	}

	VadonEditor::Core::PropertyID get_animation_channel_type_data_property_id(VadonEditor::Model::AnimationChannelType channel_type)
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::get_channel_data_property_uuid_string(channel_type));
	}
}

namespace VadonEditor::Model
{
	void AnimationChannel::set_sample_method(AnimationChannelSampleMethod sample_method)
	{
		m_sample_method = sample_method;
		emit(channel_edited(m_id));
	}

	int AnimationChannel::add_key()
	{
		AnimationKey new_key;
		new_key.id = m_key_id_counter++;
		new_key.start_time = 0.0;
		new_key.duration = 0.0;
		new_key.value = get_animation_key_default_value(m_type);

		m_keys.push_back(new_key);
		emit(channel_edited(m_id));

		return new_key.id;
	}

	void AnimationChannel::remove_key(int id)
	{
		for (auto key_it = m_keys.begin(); key_it != m_keys.end(); ++key_it)
		{
			if (key_it->id == id)
			{
				m_keys.erase(key_it);
				emit(channel_edited(m_id));
				return;
			}
		}

		// TODO: complain that key is not found
	}

	AnimationKey AnimationChannel::get_key(int id) const
	{
		for (const AnimationKey& current_key : m_keys)
		{
			if (current_key.id == id)
			{
				return current_key;
			}
		}

		// TODO: error?
		return AnimationKey{};
	}

	void AnimationChannel::set_key(const AnimationKey& key_data)
	{
		for (AnimationKey& current_key : m_keys)
		{
			if (current_key.id == key_data.id)
			{
				current_key = key_data;
				emit(channel_edited(m_id));
				return;
			}
		}
		
		// TODO: error?
	}

	void AnimationChannel::sort_keys()
	{
		std::sort(m_keys.begin(), m_keys.end());
	}

	QUuid AnimationChannel::get_sample_method_id(AnimationChannelSampleMethod sample_method)
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::get_sample_method_uuid_string(sample_method));
	}

	AnimationChannelSampleMethod AnimationChannel::get_sample_method_from_id(const QUuid& sample_method_id)
	{
		for (qsizetype sample_method_index = 0; sample_method_index < static_cast<qsizetype>(AnimationChannelSampleMethod::METHOD_COUNT); ++sample_method_index)
		{
			const AnimationChannelSampleMethod current_sample_method = static_cast<AnimationChannelSampleMethod>(sample_method_index);
			if (sample_method_id == get_sample_method_id(current_sample_method))
			{
				return current_sample_method;
			}
		}

		return AnimationChannelSampleMethod::INVALID_METHOD;
	}

	Animation::Animation(Core::Application& application)
		: m_application(application)
	{
	}

	Animation::~Animation()
	{
		clear_channels();
	}

	bool Animation::store_data(QVariant& data) const
	{
		const QUuid id_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_id_property.id);
		const QUuid sample_method_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_sample_method_property.id);
		const QUuid tag_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_tag_property.id);
		const QUuid key_times_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_key_times_property.id);

		QVariantList channel_list;

		for (AnimationChannel* current_channel : m_channels)
		{
			current_channel->sort_keys();

			QVariantList key_times_list;
			QVariantList key_data_list;

			for (const AnimationKey& current_key : current_channel->get_keys())
			{
				key_times_list.push_back(static_cast<float>(current_key.start_time));
				key_data_list.push_back(current_key.value);
			}

			const QUuid channel_type_uuid = get_animation_channel_type_uuid(current_channel->get_type());

			Core::DataObject channel_object(m_application);
			if (channel_object.init_type(channel_type_uuid) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Animation::store_data", "Failed to initialize channel object");
				return false;
			}

			const QUuid sample_method_id = AnimationChannel::get_sample_method_id(current_channel->get_sample_method());

			channel_object.set_property(id_property_id, current_channel->get_id());
			channel_object.set_property(key_times_property_id, key_times_list);
			channel_object.set_property(sample_method_property_id, sample_method_id);
			channel_object.set_property(tag_property_id, current_channel->get_tag());

			const Core::PropertyID data_property_id = get_animation_channel_type_data_property_id(current_channel->get_type());
			channel_object.set_property(data_property_id, key_data_list);

			channel_list.push_back(channel_object.export_data());
		}

		data = channel_list;
		return true;
	}

	bool Animation::load_from_resource(const Model::Resource* resource)
	{
		clear_channels();

		m_id = resource->get_info().id;

		const QUuid id_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_id_property.id);
		const QUuid sample_method_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_sample_method_property.id);
		const QUuid tag_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_tag_property.id);
		const QUuid key_times_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_key_times_property.id);

		const QVariantList channels = resource->get_data_object().get_property(get_animation_channels_property_uuid()).toList();
		for (const QVariant& current_channel_variant : channels)
		{
			Core::DataObject channel_object(m_application);
			if (channel_object.import_data(current_channel_variant.toMap()) == false)
			{
				return false;
			}

			const QUuid channel_id = channel_object.get_property(id_property_id).toUuid();
			const AnimationChannelType channel_type = get_animation_channel_type_from_uuid(channel_object.get_type_id());

			AnimationChannel* new_channel = new AnimationChannel(channel_id, channel_type, AnimationChannelMetadata{});

			const QUuid sample_method_id = channel_object.get_property(sample_method_property_id).toUuid();
			AnimationChannelSampleMethod sample_method = AnimationChannel::get_sample_method_from_id(sample_method_id);
			if (sample_method == AnimationChannelSampleMethod::INVALID_METHOD)
			{
				sample_method = AnimationChannelSampleMethod::LINEAR;
			}
			new_channel->set_sample_method(sample_method);

			const QString tag_string = channel_object.get_property(tag_property_id).toString();
			new_channel->set_tag(tag_string);

			const QVariantList key_times_list = channel_object.get_property(key_times_property_id).toList();

			const Core::PropertyID data_property_id = get_animation_channel_type_data_property_id(channel_type);
			const QVariantList data_list = channel_object.get_property(data_property_id).toList();

			Q_ASSERT_X(key_times_list.size() == data_list.size(), "VadonEditor::Model::Animation::load_data", "Invalid data");

			for (qsizetype index = 0; index < key_times_list.size(); ++index)
			{
				const int key_id = new_channel->add_key();

				AnimationKey key_data;
				key_data.id = key_id;
				key_data.start_time = key_times_list[index].toFloat();
				key_data.duration = 0.0;
				key_data.value = data_list[index];

				new_channel->set_key(key_data);
			}

			internal_add_channel(new_channel);
		}

		return true;
	}

	bool Animation::store_to_resource(Model::Resource* resource) const
	{
		QVariant animation_data;
		if (store_data(animation_data) == false)
		{
			return false;
		}

		resource->set_property(get_animation_channels_property_uuid(), animation_data);
		return true;
	}

	AnimationChannel* Animation::add_channel(AnimationChannelType type, const AnimationChannelMetadata& metadata)
	{
		QUuid new_uuid = QUuid::createUuid();
		while (find_channel(new_uuid) != nullptr)
		{
			new_uuid = QUuid::createUuid();
		}

		AnimationChannel* new_channel = new AnimationChannel(new_uuid, type, metadata);
		internal_add_channel(new_channel);

		emit(channel_added(m_id, new_uuid));

		return new_channel;
	}

	AnimationChannel* Animation::find_channel(const QUuid& channel_id) const
	{
		for (AnimationChannel* current_channel : m_channels)
		{
			if (current_channel->get_id() == channel_id)
			{
				return current_channel;
			}
		}

		return nullptr;
	}

	void Animation::remove_channel(const QUuid& channel_id)
	{
		for (qsizetype index = 0; index < m_channels.size(); ++index)
		{
			const AnimationChannel* current_channel = m_channels[index];
			if (current_channel->get_id() == channel_id)
			{
				// Notify first before removing
				emit(channel_removed(m_id, channel_id));

				m_channels.removeAt(index);
				delete current_channel;
				return;
			}
		}

		// TODO: error?
	}

	QList<QUuid> Animation::get_channel_id_list() const
	{
		QList<QUuid> channel_id_list;

		for (const AnimationChannel* current_channel : m_channels)
		{
			channel_id_list.push_back(current_channel->get_id());
		}

		return channel_id_list;
	}

	QUuid Animation::get_animation_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationSchema::c_type_uuid);
	}

	bool Animation::is_animation_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(get_animation_type_uuid(), type_id);
	}

	QUuid Animation::get_animation_library_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationLibrarySchema::c_type_uuid);
	}

	QUuid Animation::get_entity_animation_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationSchema::c_type_uuid);
	}

	QUuid Animation::get_entity_animation_entry_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_type_uuid);
	}

	QUuid Animation::get_animation_component_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationComponentSchema::c_type_uuid);
	}

	void Animation::internal_channel_edited(const QUuid& channel_id)
	{
		emit(Animation::channel_edited(m_id, channel_id));
	}

	void Animation::internal_add_channel(AnimationChannel* channel)
	{
		connect(channel, &AnimationChannel::channel_edited, this, &Animation::internal_channel_edited);
		m_channels.push_back(channel);
	}

	void Animation::clear_channels()
	{
		for (AnimationChannel* current_channel : m_channels)
		{
			delete current_channel;
		}

		m_channels.clear();
	}
}