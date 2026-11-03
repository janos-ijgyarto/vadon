#include <VadonEditor/UI/Model/Animation/ManageAnimationsDialog.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/Model/Animation/Animation.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>

namespace
{
	constexpr int c_animation_id_data_role = Qt::ItemDataRole::UserRole + 1;

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

	QUuid get_animation_list_property_id()
	{
		return VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::AnimationLibrarySchema::c_animations_property.id);
	}

	QListWidgetItem* create_animation_list_item(const QString& name, const QUuid& id, QListWidget* list_widget)
	{
		QListWidgetItem* new_item = new QListWidgetItem(name, list_widget);
		new_item->setData(c_animation_id_data_role, id);

		return new_item;
	}

	QUuid get_animation_item_id(const QListWidgetItem* item)
	{
		return item->data(c_animation_id_data_role).toUuid();
	}
}

namespace VadonEditor::UI
{
	ManageAnimationsDialog::ManageAnimationsDialog(Model::Resource* resource, QWidget* parent)
		: QDialog(parent)
		, m_resource(resource)
	{
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		Model::ResourceSystem& resource_system = m_resource->get_application().get_model_system().get_resource_system();

		QVariantList animation_list = m_resource->get_data_object().get_property(get_animation_list_property_id()).toList();
		for (const QVariant& current_animation_data : animation_list)
		{
			const QUuid current_animation_id = current_animation_data.toUuid();

			// Add to animation list
			Model::Resource* current_animation_resource = resource_system.get_resource(current_animation_id);
			Q_ASSERT_X(current_animation_resource != nullptr, "VadonEditor::UI::ManageAnimationsDialog::ManageAnimationsDialog", "Failed to get animation");

			create_animation_list_item(get_animation_resource_name(current_animation_resource), current_animation_resource->get_info().id, m_ui.animationList);
		}

		connect(m_ui.animationList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ManageAnimationsDialog::animation_selection_changed);

		update_controls();
	}

	void ManageAnimationsDialog::add_animation_clicked()
	{
		Model::Resource* new_animation = m_resource->create_embedded_resource(Model::Animation::get_animation_type_uuid());

		// Add to animation list
		create_animation_list_item(get_animation_resource_name(new_animation), new_animation->get_info().id, m_ui.animationList);

		emit(animation_added(new_animation->get_info().id));
	}

	void ManageAnimationsDialog::remove_animation_clicked()
	{
		QListWidgetItem* selected_item = get_selected_item();
		if (selected_item == nullptr)
		{
			// TODO: warning?
			return;
		}

		const QUuid removed_anim_id = get_animation_item_id(selected_item);

		delete selected_item;

		emit(animation_removed(removed_anim_id));

		update_controls();
	}

	void ManageAnimationsDialog::animation_name_edited(const QString& text)
	{
		QListWidgetItem* selected_item = get_selected_item();
		if (selected_item == nullptr)
		{
			// TODO: warning?
			return;
		}

		const QUuid animation_id = get_animation_item_id(selected_item);
		if (text.isEmpty() == false)
		{
			selected_item->setText(text);
		}
		else
		{
			selected_item->setText(get_animation_resource_name_fallback(animation_id));
		}

		Model::ResourceSystem& resource_system = m_resource->get_application().get_model_system().get_resource_system();
		Model::Resource* current_animation_resource = resource_system.get_resource(animation_id);

		current_animation_resource->set_property(Model::Resource::get_name_property_id(), text);

		emit(animation_edited(animation_id));
	}

	void ManageAnimationsDialog::animation_selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);
		update_controls();
	}

	void ManageAnimationsDialog::update_controls()
	{
		QListWidgetItem* selected_animation = get_selected_item();
		if (selected_animation != nullptr)
		{
			m_ui.animationRemoveButton->setEnabled(true);

			const QUuid animation_id = get_animation_item_id(selected_animation);

			m_ui.animationIDValueLabel->setText(VadonEditor::Utilities::uuid_to_base64_string(animation_id));

			Model::ResourceSystem& resource_system = m_resource->get_application().get_model_system().get_resource_system();
			Model::Resource* animation_resource = resource_system.get_resource(animation_id);

			QString resource_name = animation_resource->get_data_object().get_property(VadonEditor::Model::Resource::get_name_property_id()).toString();

			m_ui.animationNameEdit->setText(resource_name);
			m_ui.animationNameEdit->setEnabled(true);
		}
		else
		{
			m_ui.animationRemoveButton->setEnabled(false);

			m_ui.animationIDValueLabel->setText("N/A");
			m_ui.animationNameEdit->clear();
			m_ui.animationNameEdit->setEnabled(false);
		}
	}

	QListWidgetItem* ManageAnimationsDialog::get_selected_item() const
	{
		QList<QListWidgetItem*> selected_items = m_ui.animationList->selectedItems();
		if (selected_items.isEmpty() == false)
		{
			return selected_items.front();
		}

		return nullptr;
	}
}