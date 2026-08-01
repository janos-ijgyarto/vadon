#include <VadonEditor/Model/Animation/Animation.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

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
		default:
			return QVariant();
		}
	}

	QUuid get_animation_channel_type_uuid(VadonEditor::Model::AnimationChannelType channel_type)
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::get_channel_type_uuid_string(channel_type));
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

	Animation::~Animation()
	{
		clear_channels();
	}

	bool Animation::load_data()
	{
		clear_channels();

		const QUuid channels_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationSchema::c_channels_property.id);

		const QUuid id_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_id_property.id);
		const QUuid tag_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_tag_property.id);
		const QUuid key_times_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_key_times_property.id);

		const QVariantList channels = m_resource->get_property(channels_property_id).toList();
		for (const QVariant& current_channel_variant : channels)
		{
			Core::DataObject channel_object(m_resource->get_application());
			if (channel_object.import_data(current_channel_variant.toMap()) == false)
			{
				return false;
			}

			const QUuid channel_id = channel_object.get_property(id_property_id).toUuid();
			const AnimationChannelType channel_type = get_animation_channel_type_from_uuid(channel_object.get_type_id());

			AnimationChannel* new_channel = new AnimationChannel(channel_id, channel_type);

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

	void Animation::store_data()
	{
		const QUuid id_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationChannelSchema::c_id_property.id);
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

			Core::DataObject channel_object(m_resource->get_application());
			if (channel_object.initialize(channel_type_uuid) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::Model::Animation::store_data", "Failed to initialize channel object");
				return;
			}

			channel_object.set_property(id_property_id, current_channel->get_id());
			channel_object.set_property(key_times_property_id, key_times_list);
			channel_object.set_property(tag_property_id, current_channel->get_tag());

			const Core::PropertyID data_property_id = get_animation_channel_type_data_property_id(current_channel->get_type());
			channel_object.set_property(data_property_id, key_data_list);

			channel_list.push_back(channel_object.export_data());
		}

		const QUuid channels_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationSchema::c_channels_property.id);

		m_resource->set_property(channels_property_id, channel_list);
	}

	void Animation::save_to_resource()
	{
		// First make sure the data is up-to-date
		store_data();

		// Then save in the resource
		Model::ResourceSystem& resource_system = m_resource->get_application().get_model_system().get_resource_system();
		if (resource_system.save_resource(m_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Animation::save_to_resource", "Failed to save resource!");
		}
	}

	AnimationChannel* Animation::add_channel(AnimationChannelType type)
	{
		QUuid new_uuid = QUuid::createUuid();
		while (find_channel(new_uuid) != nullptr)
		{
			new_uuid = QUuid::createUuid();
		}

		AnimationChannel* new_channel = new AnimationChannel(new_uuid, type);
		internal_add_channel(new_channel);

		emit(channel_added(m_resource->get_info().id, new_uuid));

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
				m_channels.removeAt(index);
				delete current_channel;
				emit(channel_removed(m_resource->get_info().id, channel_id));
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

	void Animation::internal_channel_edited(const QUuid& channel_id)
	{
		emit(Animation::channel_edited(m_resource->get_info().id, channel_id));
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