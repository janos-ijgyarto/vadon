#include <VadonEditor/UI/Model/Animation/AnimationComponentWidget.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Component.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/Model/Animation/AnimationEditor.hpp>
#include <VadonEditor/UI/Model/Animation/ManageAnimationsDialog.hpp>

#include <VadonEditor/UI/Model/Scene/SelectEntityDialog.hpp>
#include <VadonEditor/UI/Model/Scene/SelectComponentPropertyDialog.hpp>

#include <Vadon/Foundation/Model/Animation/Animation.hpp>

#include <QMessageBox>

namespace
{
	QUuid get_animation_component_animation_property_id()
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationComponentSchema::c_animation_property.id);
	}

	QString get_animation_resource_name_fallback(const QUuid& animation_id)
	{
		return QString("Animation_%1").arg(VadonEditor::Utilities::uuid_to_base64_string(animation_id));
	}

	QString get_animation_resource_name(const VadonEditor::Model::Resource* anim_resource)
	{
		QString resource_name = anim_resource->get_data_object().get_property(VadonEditor::Model::Resource::get_name_property_id()).toString();
		if (resource_name.isEmpty() == true)
		{
			resource_name = get_animation_resource_name_fallback(anim_resource->get_info().id);
		}

		return resource_name;
	}

	void add_animation_to_combo_box(const VadonEditor::Model::Resource* animation, QComboBox* combo_box)
	{
		combo_box->addItem(get_animation_resource_name(animation), animation->get_info().id);
	}

	QUuid get_animation_list_property_id()
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationLibrarySchema::c_animations_property.id);
	}

	QUuid get_entity_animation_entries_property_id()
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationSchema::c_entries_property.id);
	}

	QVariantList get_animation_list_property_value(VadonEditor::Model::ResourceSystem& resource_system, const QComboBox* animation_list_combo)
	{
		QVariantList result;
		QList<VadonEditor::Model::Resource*> resource_list;

		for (int index = 0; index < animation_list_combo->count(); ++index)
		{
			const QUuid current_anim_id = animation_list_combo->itemData(index).toUuid();

			VadonEditor::Model::Resource* current_animation_resource = resource_system.get_resource(current_anim_id);
			Q_ASSERT_X(current_animation_resource != nullptr, "get_animation_list", "Failed to get animation");

			resource_list.push_back(current_animation_resource);
		}

		std::sort(resource_list.begin(), resource_list.end(),
			+[](const VadonEditor::Model::Resource* lhs, const VadonEditor::Model::Resource* rhs)
			{
				QString lhs_name = lhs->get_name();
				if (lhs_name.isEmpty() == true)
				{
					lhs_name = get_animation_resource_name_fallback(lhs->get_info().id);
				}

				QString rhs_name = rhs->get_name();
				if(rhs_name.isEmpty() == true)
				{
					rhs_name = get_animation_resource_name_fallback(rhs->get_info().id);
				}

				return lhs_name < rhs_name;
			}
		);

		for (const VadonEditor::Model::Resource* current_resource : resource_list)
		{
			result.push_back(current_resource->get_info().id);
		}

		return result;
	}

	VadonEditor::Model::AnimationChannelType get_base_type_animation_channel_type(::Vadon::Foundation::BaseType base_type)
	{
		switch (base_type)
		{
		case ::Vadon::Foundation::BaseType::INT32:
			return VadonEditor::Model::AnimationChannelType::INT;
		case ::Vadon::Foundation::BaseType::UINT32:
			return VadonEditor::Model::AnimationChannelType::UINT;
		case ::Vadon::Foundation::BaseType::FLOAT:
			return VadonEditor::Model::AnimationChannelType::FLOAT;
		case ::Vadon::Foundation::BaseType::VECTOR2:
			return VadonEditor::Model::AnimationChannelType::VECTOR2;
		case ::Vadon::Foundation::BaseType::VECTOR2I:
			return VadonEditor::Model::AnimationChannelType::VECTOR2I;
		case ::Vadon::Foundation::BaseType::VECTOR3:
			return VadonEditor::Model::AnimationChannelType::VECTOR3;
		case ::Vadon::Foundation::BaseType::VECTOR3I:
			return VadonEditor::Model::AnimationChannelType::VECTOR3I;
		case ::Vadon::Foundation::BaseType::VECTOR4:
			return VadonEditor::Model::AnimationChannelType::VECTOR4;
		case ::Vadon::Foundation::BaseType::COLORRGBA:
			return VadonEditor::Model::AnimationChannelType::COLORRGBA;
		}

		return VadonEditor::Model::AnimationChannelType::INVALID_TYPE;
	}
}

namespace VadonEditor::UI
{
	bool EntityAnimationChannelEntry::load_data(Core::Application& application, const QVariantMap& properties)
	{
		Core::DataObject entry_data_obj(application);
		if (entry_data_obj.init_type(Model::Animation::get_entity_animation_entry_type_uuid()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityAnimationChannelEntry::load_data", "Failed to initialize data object");
			return false;
		}

		entry_data_obj.load_properties(properties);

		animation_id = entry_data_obj.get_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_animation_property.id)).toUuid();
		channel_id = entry_data_obj.get_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_channel_property.id)).toUuid();
		entity_id = entry_data_obj.get_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_entity_property.id)).toUuid();
		component_id = entry_data_obj.get_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_component_property.id)).toUuid();
		property_id = entry_data_obj.get_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_property_id_property.id)).toUuid();

		return true;
	}

	bool EntityAnimationChannelEntry::store_data(Core::Application& application, QVariantMap& properties) const
	{
		Core::DataObject entry_data_obj(application);
		if (entry_data_obj.init_type(Model::Animation::get_entity_animation_entry_type_uuid()) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::EntityAnimationChannelEntry::store_data", "Failed to initialize data object");
			return false;
		}

		entry_data_obj.set_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_animation_property.id), animation_id);
		entry_data_obj.set_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_channel_property.id), channel_id);
		entry_data_obj.set_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_entity_property.id), entity_id);
		entry_data_obj.set_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_component_property.id), component_id);
		entry_data_obj.set_property(VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::EntityAnimationEntrySchema::c_property_id_property.id), property_id);

		properties = entry_data_obj.get_property_map();

		return true;
	}

	AnimationComponentWidget::AnimationComponentWidget(Model::Scene* owner_scene, Model::Entity* entity, Model::Component* component, QWidget* parent)
		: QWidget(parent)
		, m_owner_scene(owner_scene)
		, m_entity(entity)
		, m_component(component)
		, m_focused_editor(nullptr)
	{
		Q_ASSERT_X(owner_scene != nullptr, "VadonEditor::UI::AnimationComponentWidget::AnimationComponentWidget", "Must have valid owner scene");
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		{
			const Model::EntityModel& entity_model = owner_scene->get_entity_model();
			QList<QUuid> entity_id_path = entity_model.get_entity_path(entity->get_id());
			Q_ASSERT_X(entity_id_path.isEmpty() == false, "VadonEditor::UI::AnimationComponentWidget::AnimationComponentWidget", "Invalid entity path");

			QStringList entity_label_path;
			for (const QUuid& current_id : entity_id_path)
			{
				const Model::Entity* current_entity = entity_model.find_entity_by_id(current_id);
				Q_ASSERT_X(current_entity != nullptr, "VadonEditor::UI::AnimationComponentWidget::AnimationComponentWidget", "Invalid entity path");
				entity_label_path.push_back(current_entity->get_label());
			}

			m_ui.entityPathLabel->setText(entity_label_path.join('/'));
		}

		m_ui.resourceToolButton->addAction(m_ui.actionCreateAnimation);
		m_ui.resourceToolButton->addAction(m_ui.actionClear);

		Model::Resource* animation_resource = get_entity_animation_resource();
		if (animation_resource != nullptr)
		{
			// Create LUT for animation channel entries
			QVariantList animation_list = animation_resource->get_data_object().get_property(get_animation_list_property_id()).toList();
			for (const QVariant& current_animation_data : animation_list)
			{
				const QUuid current_animation_id = current_animation_data.toUuid();
				m_channel_entry_lookup.insert(current_animation_id, QHash<QUuid, EntityAnimationChannelEntry>());
			}

			const QVariantList animation_entries = animation_resource->get_data_object().get_property(get_entity_animation_entries_property_id()).toList();
			for (const QVariant& current_entry_data : animation_entries)
			{
				EntityAnimationChannelEntry current_entry;
				if (current_entry.load_data(m_owner_scene->get_application(), current_entry_data.toMap()) == false)
				{
					// TODO: warning/error?
					continue;
				}

				auto entry_it = m_channel_entry_lookup.find(current_entry.animation_id);
				if (entry_it == m_channel_entry_lookup.end())
				{
					// TODO: warning/error?
					continue;
				}

				entry_it.value().insert(current_entry.channel_id, current_entry);
			}

			generate_widgets();
		}
		else
		{
			clear_widgets();
		}
	}

	void AnimationComponentWidget::manage_animations_clicked()
	{
		Model::Resource* animation_resource = get_entity_animation_resource();
		if (animation_resource == nullptr)
		{
			// TODO: error?
			return;
		}

		ManageAnimationsDialog* manage_anims_dialog = new ManageAnimationsDialog(animation_resource, this);

		connect(manage_anims_dialog, &ManageAnimationsDialog::animation_added, this, &AnimationComponentWidget::animation_added);
		connect(manage_anims_dialog, &ManageAnimationsDialog::animation_edited, this, &AnimationComponentWidget::animation_edited);
		connect(manage_anims_dialog, &ManageAnimationsDialog::animation_removed, this, &AnimationComponentWidget::animation_removed);

		manage_anims_dialog->show();
	}

	void AnimationComponentWidget::animation_selected(int index)
	{
		const QUuid selected_animation_id = m_ui.animationComboBox->itemData(index).toUuid();
		open_animation(selected_animation_id);
	}

	void AnimationComponentWidget::create_animation_triggered()
	{
		create_resource();
	}

	void AnimationComponentWidget::clear_animation_triggered()
	{
		remove_resource();
	}

	void AnimationComponentWidget::new_channel_requested(const QUuid& animation_id)
	{
		if (Utilities::is_uuid_valid(m_new_channel_request_id) == true)
		{
			qCritical() << "Channel request already in progress!";
			return;
		}

		auto editor_it = m_animation_editor_lookup.find(animation_id);
		if (editor_it == m_animation_editor_lookup.end())
		{
			qCritical() << "Cannot find animation editor for" << animation_id;
			return;
		}

		m_new_channel_request_id = animation_id;

		SelectEntityDialog* select_entity_dialog = new SelectEntityDialog(m_owner_scene, this);

		connect(select_entity_dialog, &SelectEntityDialog::rejected, this, &AnimationComponentWidget::clear_channel_request);
		connect(select_entity_dialog, &SelectEntityDialog::entity_selected, this, &AnimationComponentWidget::new_channel_entity_selected);

		select_entity_dialog->show();
	}

	void AnimationComponentWidget::remove_channel_requested(const QUuid& animation_id, const QUuid& channel_id)
	{
		auto editor_it = m_animation_editor_lookup.find(animation_id);
		if (editor_it == m_animation_editor_lookup.end())
		{
			qCritical() << "Cannot find animation editor for" << animation_id;
			return;
		}

		const AnimationEditorEntry& editor_entry = editor_it.value();
		editor_entry.model->remove_channel(channel_id);
	}

	void AnimationComponentWidget::new_channel_entity_selected(const QUuid& entity_id)
	{
		const Model::Entity* entity = m_owner_scene->get_entity_model().find_entity_by_id(entity_id);
		if (entity == nullptr)
		{
			QMessageBox::critical(this, "Scene System Error", QString("Cannot find entity %1 in scene %2").arg(entity_id.toString()).arg(m_owner_scene->get_id().toString()));
			return;
		}

		SelectComponentPropertyDialog* component_property_dialog = new SelectComponentPropertyDialog(entity, this);

		connect(component_property_dialog, &SelectComponentPropertyDialog::rejected, this, &AnimationComponentWidget::clear_channel_request);
		connect(component_property_dialog, &SelectComponentPropertyDialog::component_property_selected, this, &AnimationComponentWidget::new_channel_component_property_selected);

		component_property_dialog->show();
	}

	void AnimationComponentWidget::new_channel_component_property_selected(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id)
	{
		const QUuid animation_id = m_new_channel_request_id;
		clear_channel_request();

		auto editor_it = m_animation_editor_lookup.find(animation_id);
		if (editor_it == m_animation_editor_lookup.end())
		{
			qCritical() << "Cannot find animation editor for" << animation_id;
			return;
		}

		const Core::DataSchema& data_schema = m_owner_scene->get_application().get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(component_id);
		Q_ASSERT_X(type_data != nullptr, "VadonEditor::UI::AnimationComponentWidget::new_channel_component_property_selected", "Cannot find type data");

		const Core::PropertyData* property_data = type_data->find_property_data(property_id);
		Q_ASSERT_X(property_data != nullptr, "VadonEditor::UI::AnimationComponentWidget::new_channel_component_property_selected", "Cannot find property data");

		AnimationEditorEntry& editor_entry = editor_it.value();

		EntityAnimationChannelEntry new_channel_entry;
		new_channel_entry.animation_id = animation_id;
		new_channel_entry.entity_id = entity_id;
		new_channel_entry.component_id = component_id;
		new_channel_entry.property_id = property_id;

		switch (Core::PropertyData::get_category(property_data->get_root_type()))
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			const ::Vadon::Foundation::BaseType base_type = Core::TypeData::get_base_type(property_data->get_root_type());
			const Model::AnimationChannelType channel_type = get_base_type_animation_channel_type(base_type);

			if (channel_type == Model::AnimationChannelType::INVALID_TYPE)
			{
				qCritical() << "Cannot create animation channel for property" << property_id << "(invalid base type)";
				return;
			}

			const Model::AnimationChannel* new_channel = editor_entry.model->add_channel(channel_type);

			new_channel_entry.channel_id = new_channel->get_id();
		}
			break;
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			const Model::AnimationChannelType channel_type = Model::AnimationChannelType::UUID;

			Q_ASSERT_X(property_data->type_list.size() > 1, "VadonEditor::UI::AnimationComponentWidget::new_channel_component_property_selected", "Missing resource type!");

			Model::AnimationChannelMetadata channel_metadata;
			channel_metadata.resource_type = property_data->type_list[1];

			const Model::AnimationChannel* new_channel = editor_entry.model->add_channel(channel_type, channel_metadata);

			new_channel_entry.channel_id = new_channel->get_id();
		}
			break;
		default:
		{
			qCritical() << "Cannot create animation channel for property" << property_id << "(invalid property category)";
			return;
		}
		}

		add_channel_entry(new_channel_entry);
	}

	void AnimationComponentWidget::clear_channel_request()
	{
		m_new_channel_request_id = QUuid();
	}

	void AnimationComponentWidget::animation_added(const QUuid& animation_id)
	{
		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		Model::Resource* added_animation = resource_system.get_resource(animation_id);
		Q_ASSERT_X(added_animation != nullptr, "VadonEditor::UI::AnimationComponentWidget::animation_added", "Failed to get animation");

		add_animation_to_combo_box(added_animation, m_ui.animationComboBox);

		Q_ASSERT_X(m_channel_entry_lookup.find(animation_id) == m_channel_entry_lookup.end(), "VadonEditor::UI::AnimationComponentWidget::animation_added", "Animation already in lookup");
		m_channel_entry_lookup.insert(animation_id, QHash<QUuid, EntityAnimationChannelEntry>());

		update_animation_list_property_data();
	}

	void AnimationComponentWidget::animation_edited(const QUuid& animation_id)
	{
		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		Model::Resource* edited_animation = resource_system.get_resource(animation_id);

		for (int index = 0; index < m_ui.animationComboBox->count(); ++index)
		{
			const QUuid current_item_id = m_ui.animationComboBox->itemData(index).toUuid();
			if (animation_id == current_item_id)
			{
				m_ui.animationComboBox->setItemText(index, get_animation_resource_name(edited_animation));

				update_animation_list_property_data();
				return;
			}
		}

		Q_UNREACHABLE();
	}

	void AnimationComponentWidget::animation_removed(const QUuid& animation_id)
	{
		for (int index = 0; index < m_ui.animationComboBox->count(); ++index)
		{
			const QUuid current_item_id = m_ui.animationComboBox->itemData(index).toUuid();
			if (animation_id == current_item_id)
			{
				m_ui.animationComboBox->removeItem(index);

				auto anim_editor_it = m_animation_editor_lookup.find(animation_id);
				if (anim_editor_it != m_animation_editor_lookup.end())
				{
					AnimationEditorEntry& editor_entry = anim_editor_it.value();
					editor_entry.editor->deleteLater();
					editor_entry.model->deleteLater();
					m_animation_editor_lookup.erase(anim_editor_it);
				}

				break;
			}
		}

		if (m_focused_editor != nullptr)
		{
			if (m_focused_editor->get_animation().get_id() == animation_id)
			{
				m_focused_editor = nullptr;
			}
		}

		auto channel_entry_it = m_channel_entry_lookup.find(animation_id);
		Q_ASSERT_X(m_channel_entry_lookup.find(animation_id) != m_channel_entry_lookup.end(), "VadonEditor::UI::AnimationComponentWidget::animation_removed", "Animation missing from lookup");

		m_channel_entry_lookup.erase(channel_entry_it);

		update_animation_list_property_data();
	}

	void AnimationComponentWidget::animation_channel_added(const QUuid& animation_id, const QUuid& channel_id)
	{
		Q_UNUSED(channel_id);
		store_animation_data(animation_id);
		update_animation_entries_property_data();
	}

	void AnimationComponentWidget::animation_channel_edited(const QUuid& animation_id, const QUuid& channel_id)
	{
		Q_UNUSED(channel_id);
		store_animation_data(animation_id);
	}

	void AnimationComponentWidget::animation_channel_removed(const QUuid& animation_id, const QUuid& channel_id)
	{
		auto editor_it = m_animation_editor_lookup.find(animation_id);
		if (editor_it == m_animation_editor_lookup.end())
		{
			qCritical() << "Cannot find animation editor for" << animation_id;
			return;
		}

		remove_channel_entry(animation_id, channel_id);

		store_animation_data(animation_id);
		update_animation_entries_property_data();
	}

	void AnimationComponentWidget::create_resource()
	{
		// Clear previous resource
		remove_resource();

		Model::Resource* new_resource = m_owner_scene->get_resource()->create_embedded_resource(Model::Animation::get_entity_animation_type_uuid());

		const QUuid entity_anim_property_id = get_animation_component_animation_property_id();
		m_component->set_property(entity_anim_property_id, new_resource->get_info().id);

		generate_widgets();
	}

	void AnimationComponentWidget::remove_resource()
	{
		Model::Resource* prev_resource = get_entity_animation_resource();
		if (prev_resource == nullptr)
		{
			return;
		}

		const QUuid entity_anim_property_id = get_animation_component_animation_property_id();
		m_component->set_property(entity_anim_property_id, QUuid());

		clear_widgets();

		if (prev_resource->is_embedded() == true)
		{
			// Delete embedded resource
			delete prev_resource;
		}
		prev_resource = nullptr;

		m_channel_entry_lookup.clear();
	}

	void AnimationComponentWidget::open_animation(const QUuid& animation_id)
	{
		auto editor_it = m_animation_editor_lookup.find(animation_id);
		if (editor_it != m_animation_editor_lookup.end())
		{
			AnimationEditorEntry& new_focused_entry = editor_it.value();
			if (new_focused_entry.editor != m_focused_editor)
			{
				m_focused_editor->setVisible(false);
				new_focused_entry.editor->setVisible(true);

				m_focused_editor = new_focused_entry.editor;
			}
			return;
		}

		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		Model::Resource* animation_resource = resource_system.get_resource(animation_id);
		Q_ASSERT_X(animation_resource != nullptr, "VadonEditor::UI::AnimationComponentWidget::open_animation", "Failed to get animation resource");

		// Create model object
		Model::Animation* new_animation_model = new Model::Animation(application);
		if (new_animation_model->load_from_resource(animation_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::AnimationComponentWidget::open_animation", "Failed to load animation data");
			new_animation_model->deleteLater();
			return;
		}

		auto channel_entry_it = m_channel_entry_lookup.find(animation_id);
		Q_ASSERT_X(channel_entry_it != m_channel_entry_lookup.end(), "VadonEditor::UI::AnimationComponentWidget::open_animation", "Cannot ");

		const auto& channel_entries = channel_entry_it.value();
		for (auto current_entry_it = channel_entries.begin(); current_entry_it != channel_entries.end(); ++current_entry_it)
		{
			const EntityAnimationChannelEntry& current_entry = current_entry_it.value();
			Model::AnimationChannel* entry_channel = new_animation_model->find_channel(current_entry.channel_id);
			if (entry_channel == nullptr)
			{
				// TODO: stale channel, remove it!
				continue;
			}

			const Model::Entity* entity = m_owner_scene->get_entity_model().find_entity_by_id(current_entry.entity_id);
			if (entity == nullptr)
			{
				// TODO: stale channel, remove it!
				continue;
			}

			const Model::Component* component = entity->get_component(current_entry.component_id);
			if (component == nullptr)
			{
				// TODO: stale channel, remove it!
				continue;
			}

			const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
			const Core::TypeData* type_data = data_schema.find_type_data(current_entry.component_id);
			Q_ASSERT_X(type_data != nullptr, "VadonEditor::UI::AnimationComponentWidget::open_animation", "Cannot find type data");

			const Core::PropertyData* property_data = type_data->find_property_data(current_entry.property_id);
			Q_ASSERT_X(property_data != nullptr, "VadonEditor::UI::AnimationComponentWidget::open_animation", "Cannot find property data");

			switch (Core::PropertyData::get_category(property_data->get_root_type()))
			{
			case ::Vadon::Foundation::Property::Category::RESOURCE:
			{
				Q_ASSERT_X(property_data->type_list.size() > 1, "VadonEditor::UI::AnimationComponentWidget::open_animation", "Missing resource type!");

				Model::AnimationChannelMetadata channel_metadata;
				channel_metadata.resource_type = property_data->type_list[1];

				entry_channel->set_metadata(channel_metadata);
			}
			break;
			}
		}

		connect(new_animation_model, &Model::Animation::channel_added, this, &AnimationComponentWidget::animation_channel_added);
		connect(new_animation_model, &Model::Animation::channel_edited, this, &AnimationComponentWidget::animation_channel_edited);
		connect(new_animation_model, &Model::Animation::channel_removed, this, &AnimationComponentWidget::animation_channel_removed);

		AnimationEditor* new_animation_editor = new AnimationEditor(*new_animation_model, this);
		if (m_focused_editor != nullptr)
		{
			m_focused_editor->setVisible(false);
		}

		connect(new_animation_editor, &AnimationEditor::new_channel_requested, this, &AnimationComponentWidget::new_channel_requested);
		connect(new_animation_editor, &AnimationEditor::remove_channel_requested, this, &AnimationComponentWidget::remove_channel_requested);

		m_focused_editor = new_animation_editor;
		m_ui.mainLayout->addWidget(new_animation_editor, 1);

		AnimationEditorEntry new_editor_entry;
		new_editor_entry.model = new_animation_model;
		new_editor_entry.editor = new_animation_editor;

		m_animation_editor_lookup.insert(animation_id, new_editor_entry);
	}

	void AnimationComponentWidget::add_channel_entry(const EntityAnimationChannelEntry& entry)
	{
		auto entry_lookup_it = m_channel_entry_lookup.find(entry.animation_id);
		if (entry_lookup_it != m_channel_entry_lookup.end())
		{
			auto& entry_lookup = entry_lookup_it.value();
			Q_ASSERT_X(entry_lookup.find(entry.channel_id) == entry_lookup.end(), "VadonEditor::UI::AnimationComponentWidget::add_channel_entry", "Entry already present");

			entry_lookup.insert(entry.channel_id, entry);
			update_animation_entries_property_data();
			return;
		}

		// TODO: error?
	}

	void AnimationComponentWidget::remove_channel_entry(const QUuid& animation_id, const QUuid& channel_id)
	{
		auto entry_lookup_it = m_channel_entry_lookup.find(animation_id);
		if (entry_lookup_it != m_channel_entry_lookup.end())
		{
			auto& entry_lookup = entry_lookup_it.value();
			auto channel_it = entry_lookup.find(channel_id);
			if (channel_it != entry_lookup.end())
			{
				entry_lookup.erase(channel_it);
				update_animation_entries_property_data();
				return;
			}
		}

		// TODO: error?
	}

	void AnimationComponentWidget::generate_widgets()
	{
		Model::Resource* entity_anim_resource = get_entity_animation_resource();
		if (entity_anim_resource == nullptr)
		{
			return;
		}

		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		QVariantList animation_list = entity_anim_resource->get_data_object().get_property(get_animation_list_property_id()).toList();
		for (const QVariant& current_animation_data : animation_list)
		{
			const QUuid current_animation_id = current_animation_data.toUuid();

			// Add to animation list
			Model::Resource* current_animation_resource = resource_system.get_resource(current_animation_id);
			Q_ASSERT_X(current_animation_resource != nullptr, "VadonEditor::UI::AnimationComponentWidget::generate_widgets", "Failed to get animation");

			add_animation_to_combo_box(current_animation_resource, m_ui.animationComboBox);
		}

		m_ui.animationComboBox->setEnabled(true);
		m_ui.manageAnimationsButton->setEnabled(true);
		
		if (m_ui.animationComboBox->count() > 0)
		{
			m_ui.animationComboBox->setCurrentIndex(0);
			animation_selected(0);
		}

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(Model::Animation::get_entity_animation_type_uuid());
		Q_ASSERT_X(type_data != nullptr, "VadonEditor::UI::AnimationComponentWidget::generate_widgets", "Cannot find type data");

		QString type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (type_name.isEmpty())
		{
			type_name = QString("Animation Library %1").arg(Model::Animation::get_entity_animation_type_uuid().toString());
		}

		m_ui.resourceHeaderLabel->setText(type_name);
	}

	void AnimationComponentWidget::clear_widgets()
	{
		for (auto editor_it = m_animation_editor_lookup.begin(); editor_it != m_animation_editor_lookup.end(); ++editor_it)
		{
			AnimationEditorEntry& editor_entry = editor_it.value();
			editor_entry.editor->deleteLater();
			editor_entry.model->deleteLater();
		}

		m_animation_editor_lookup.clear();

		m_ui.resourceHeaderLabel->setText("N/A");

		m_ui.animationComboBox->clear();
		m_ui.animationComboBox->setEnabled(false);

		m_ui.manageAnimationsButton->setEnabled(false);

		m_focused_editor = nullptr;
	}

	void AnimationComponentWidget::update_animation_list_property_data() const
	{
		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		Model::Resource* entity_animation = get_entity_animation_resource();
		Q_ASSERT_X(entity_animation != nullptr, "VadonEditor::UI::AnimationComponentWidget::update_animation_list_property_data", "Entity animation not set");

		entity_animation->set_property(get_animation_list_property_id(), get_animation_list_property_value(resource_system, m_ui.animationComboBox));

		notify_component_modified();
	}

	void AnimationComponentWidget::update_animation_entries_property_data() const
	{
		Model::Resource* entity_animation = get_entity_animation_resource();
		Q_ASSERT_X(entity_animation != nullptr, "VadonEditor::UI::AnimationComponentWidget::update_animation_list_property_data", "Entity animation not set");

		QList<EntityAnimationChannelEntry> channel_entries;
		for (auto animation_it = m_channel_entry_lookup.begin(); animation_it != m_channel_entry_lookup.end(); ++animation_it)
		{
			auto current_animation_lookup = animation_it.value();
			for (auto channel_it = current_animation_lookup.begin(); channel_it != current_animation_lookup.end(); ++channel_it)
			{
				channel_entries.push_back(channel_it.value());
			}
		}

		std::sort(channel_entries.begin(), channel_entries.end(),
			+[](const EntityAnimationChannelEntry& lhs, const EntityAnimationChannelEntry& rhs)
			{
				if (lhs.animation_id == rhs.animation_id)
				{
					if (lhs.channel_id == rhs.channel_id)
					{
						if (lhs.entity_id == rhs.entity_id)
						{
							if (lhs.component_id == rhs.component_id)
							{
								return lhs.property_id < rhs.property_id;
							}

							return lhs.component_id < rhs.component_id;
						}

						return lhs.entity_id < rhs.entity_id;
					}

					return lhs.channel_id < rhs.channel_id;
				}

				return lhs.animation_id < rhs.animation_id;
			}
		);

		QVariantList channel_entries_data;
		for (const EntityAnimationChannelEntry& current_entry : channel_entries)
		{
			QVariantMap current_entry_properties;
			if (current_entry.store_data(m_component->get_application(), current_entry_properties) == false)
			{
				Q_ASSERT_X(entity_animation != nullptr, "VadonEditor::UI::AnimationComponentWidget::update_animation_entries_property_data", "Failed to store data");
				return;
			}

			channel_entries_data.push_back(current_entry_properties);
		}

		entity_animation->set_property(get_entity_animation_entries_property_id(), channel_entries_data);

		notify_component_modified();
	}

	void AnimationComponentWidget::notify_component_modified() const
	{
		m_component->notify_modified();
	}

	Model::Resource* AnimationComponentWidget::get_entity_animation_resource() const
	{
		const QUuid entity_anim_property_id = get_animation_component_animation_property_id();

		const Core::DataObject& component_data = m_component->get_data_object();
		const QUuid entity_animation_id = component_data.get_property(entity_anim_property_id).toUuid();

		if (Utilities::is_uuid_valid(entity_animation_id) == false)
		{
			return nullptr;
		}

		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		Model::Resource* animation_resource = resource_system.get_resource(entity_animation_id);
		Q_ASSERT_X(animation_resource != nullptr, "VadonEditor::UI::AnimationComponentWidget::generate_widgets", "Failed to get animation resource");

		return animation_resource;
	}

	Model::Resource* AnimationComponentWidget::find_animation_in_entity_animation(const QUuid& animation_id) const
	{
		Model::Resource* entity_animation = get_entity_animation_resource();
		if (entity_animation == nullptr)
		{
			return nullptr;
		}

		Core::Application& application = m_component->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();

		QVariantList animation_list = entity_animation->get_data_object().get_property(get_animation_list_property_id()).toList();
		for (const QVariant& current_animation_data : animation_list)
		{
			const QUuid current_animation_id = current_animation_data.toUuid();

			if (current_animation_id == animation_id)
			{
				Model::Resource* current_animation_resource = resource_system.get_resource(current_animation_id);
				Q_ASSERT_X(current_animation_resource != nullptr, "VadonEditor::UI::AnimationComponentWidget::find_animation_in_entity_animation", "Failed to get animation");

				return current_animation_resource;
			}
		}

		return nullptr;
	}

	void AnimationComponentWidget::store_animation_data(const QUuid& animation_id) const
	{
		const auto entry_it = m_animation_editor_lookup.find(animation_id);
		if (entry_it == m_animation_editor_lookup.end())
		{
			return;
		}

		const AnimationEditorEntry& editor_entry = entry_it.value();

		Model::Resource* animation_resource = find_animation_in_entity_animation(animation_id);
		if (animation_resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::AnimationComponentWidget::store_animation_data", "Cannot find resource");
			return;
		}

		if (editor_entry.model->store_to_resource(animation_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::AnimationComponentWidget::store_animation_data", "Failed to store data");
		}

		notify_component_modified();
	}
}