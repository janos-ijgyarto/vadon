#include <VadonEditor/UI/Model/Scene/SelectComponentPropertyDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>

#include <QPushButton>

namespace
{
	enum class ComponentPropertyItemDataRole
	{
		NODE_TYPE = Qt::ItemDataRole::UserRole + 1,
		ID
	};

	enum class ComponentPropertyNodeType
	{
		COMPONENT,
		PROPERTY
	};

	QStandardItem* create_component_property_tree_standard_item(const QString& label, ComponentPropertyNodeType type, const QUuid& id)
	{
		QStandardItem* new_item = new QStandardItem(label);
		new_item->setCheckable(false);
		new_item->setEditable(false);
		new_item->setDragEnabled(false);

		new_item->setData(static_cast<int>(type), static_cast<int>(ComponentPropertyItemDataRole::NODE_TYPE));
		new_item->setData(id, static_cast<int>(ComponentPropertyItemDataRole::ID));

		return new_item;
	}
}

namespace VadonEditor::UI
{
	SelectComponentPropertyDialog::SelectComponentPropertyDialog(const Model::Entity* owner_entity, QWidget* parent)
		: QDialog(parent)
		, m_owner_entity(owner_entity)
	{
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		const Core::DataSchema& data_schema = owner_entity->get_application().get_project_manager().get_project_data_schema();

		const QList<QUuid> component_ids = owner_entity->get_component_id_list();
		for (const QUuid& current_component_id : component_ids)
		{
			const Core::TypeData* type_data = data_schema.find_type_data(current_component_id);
			Q_ASSERT_X(type_data != nullptr, "VadonEditor::UI::SelectComponentPropertyDialog::SelectComponentPropertyDialog", "Cannot find type data");

			QList<const Core::TypeData*> type_parent_list;
			{
				const Core::TypeData* parent_type = type_data;
				while (parent_type != nullptr)
				{
					type_parent_list.push_back(parent_type);
					parent_type = data_schema.find_type_data(Utilities::vadon_uuid_to_qt_uuid(parent_type->info.base_id));
				}
			}

			QString component_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
			if (component_type_name.isEmpty())
			{
				component_type_name = QString("Component type %1").arg(Utilities::vadon_uuid_to_qt_uuid(type_data->info.id).toString());
			}

			QStandardItem* component_item = create_component_property_tree_standard_item(component_type_name, ComponentPropertyNodeType::COMPONENT, current_component_id);

			for (const Core::TypeData* current_type : type_parent_list)
			{
				for (const ::Vadon::Foundation::UUID& property_uuid : current_type->property_list)
				{
					const QUuid property_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(property_uuid);
					const Core::PropertyData* property_data = current_type->find_property_data(property_qt_uuid);

					if (property_data->flags & ::Vadon::Foundation::CommonPropertyMetadata::Flags::EDITOR_HIDDEN)
					{
						// Skip properties that are hidden
						continue;
					}

					QString property_name = property_data->find_metadata(::Vadon::Foundation::CommonPropertyMetadata::NAME);
					if (property_name.isEmpty())
					{
						property_name = QString("Property %1").arg(property_qt_uuid.toString());
					}

					QStandardItem* property_item = create_component_property_tree_standard_item(property_name, ComponentPropertyNodeType::PROPERTY, property_qt_uuid);
					component_item->appendRow(property_item);
				}
			}

			m_component_property_model.invisibleRootItem()->appendRow(component_item);
		}

		m_ui.componentPropertyTree->setModel(&m_component_property_model);

		connect(m_ui.componentPropertyTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectComponentPropertyDialog::selection_changed);

		update_controls();
	}

	void SelectComponentPropertyDialog::ok_clicked()
	{
		QStandardItem* item = get_selected_item();
		finalize_selection(item);
	}

	void SelectComponentPropertyDialog::selection_changed(const QItemSelection& selected, const QItemSelection& deselected)
	{
		Q_UNUSED(selected);
		Q_UNUSED(deselected);
		update_controls();
	}

	void SelectComponentPropertyDialog::tree_item_double_clicked(const QModelIndex& index)
	{
		QStandardItem* item = m_component_property_model.itemFromIndex(index);
		finalize_selection(item);
	}

	void SelectComponentPropertyDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);

		const QStandardItem* selected_item = get_selected_item();
		if (selected_item != nullptr)
		{
			const int node_type_int = selected_item->data(static_cast<int>(ComponentPropertyItemDataRole::NODE_TYPE)).toInt();
			if (static_cast<ComponentPropertyNodeType>(node_type_int) == ComponentPropertyNodeType::PROPERTY)
			{
				ok_button->setEnabled(true);
				return;
			}
		}

		ok_button->setEnabled(false);
	}

	QStandardItem* SelectComponentPropertyDialog::get_selected_item() const
	{
		QItemSelectionModel* selection_model = m_ui.componentPropertyTree->selectionModel();
		if (selection_model->hasSelection() == false)
		{
			return nullptr;
		}

		const QModelIndexList selected_index_list = selection_model->selectedIndexes();
		if (selected_index_list.isEmpty() == true)
		{
			return nullptr;
		}

		const QModelIndex selected_index = selected_index_list.front();
		return m_component_property_model.itemFromIndex(selected_index);
	}

	void SelectComponentPropertyDialog::finalize_selection(const QStandardItem* item)
	{
		if (item == nullptr)
		{
			// TODO: error!
			return;
		}

		const int node_type_int = item->data(static_cast<int>(ComponentPropertyItemDataRole::NODE_TYPE)).toInt();
		if (static_cast<ComponentPropertyNodeType>(node_type_int) != ComponentPropertyNodeType::PROPERTY)
		{
			// TODO: error!
			return;
		}

		const QUuid property_id = item->data(static_cast<int>(ComponentPropertyItemDataRole::ID)).toUuid();
		const QUuid component_id = item->parent()->data(static_cast<int>(ComponentPropertyItemDataRole::ID)).toUuid();

		emit(component_property_selected(m_owner_entity->get_id(), component_id, property_id));
		accept();
	}
}