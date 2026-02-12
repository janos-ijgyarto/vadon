#include <VadonEditor/UI/Project/DataSchemaDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/TypeInfo/Metadata.hpp>

#include <QLabel>

namespace
{
	constexpr Qt::ItemDataRole c_tree_uuid_role = Qt::ItemDataRole::UserRole;
}

namespace VadonEditor::UI
{
	DataSchemaDialog::DataSchemaDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_schema_model({ "Name" })
	{
		setAttribute(Qt::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		initialize();
	}

	DataSchemaDialog::~DataSchemaDialog()
	{
	}

	void DataSchemaDialog::type_selection_changed()
	{
		// Clear all previous widgets
		{
			const int spacer_index = m_ui.propertiesMainVBox->indexOf(m_ui.propertiesSpacer);
			for (int child_index = 0; child_index < m_ui.propertiesMainVBox->count(); ++child_index)
			{
				if (child_index == spacer_index)
				{
					continue;
				}
				m_ui.propertiesMainVBox->itemAt(child_index)->widget()->deleteLater();
			}
		}

		QList<QTreeWidgetItem*> selected_items = m_ui.schemaTree->selectedItems();
		if (selected_items.isEmpty() == true)
		{
			return;
		}

		QTreeWidgetItem* selected_type_item = selected_items.front();
		QVariant item_data = selected_type_item->data(0, c_tree_uuid_role);
		if (item_data.isValid() == false)
		{
			return;
		}

		const QUuid type_qt_uuid = item_data.toUuid();
		const Vadon::Foundation::UUID type_uuid = VadonEditor::Utilities::qt_uuid_to_vadon_uuid(type_qt_uuid);

		Core::ProjectManager& project_manager = m_application.get_project_manager();
		const Core::DataSchema& project_data_schema = project_manager.get_project_data_schema();

		const VadonEditor::Core::TypeData* type_data = project_data_schema.find_type_data(type_uuid);
		Vadon::Foundation::UUID parent_type_uuid = type_data->info.id;
		while (parent_type_uuid.is_valid() == true)
		{
			QGroupBox* property_group_box = new QGroupBox();
			const VadonEditor::Core::TypeData* parent_type_data = project_data_schema.find_type_data(parent_type_uuid);

			property_group_box->setTitle(parent_type_data->get_name());

			QGridLayout* property_grid = new QGridLayout;

			int current_row = 0;
			for (const ::Vadon::Foundation::UUID& property_uuid : parent_type_data->property_list)
			{
				const VadonEditor::Core::PropertyData* property_data = parent_type_data->find_property_data(property_uuid);
				property_grid->addWidget(new QLabel(property_data->get_name()), current_row, 0);

				const VadonEditor::Core::TypeData* property_type_data = project_data_schema.find_type_data(property_data->get_data_type());
				QString property_label = property_type_data->get_name();
				if (property_data->get_category() == Core::PropertyCategory::ARRAY)
				{
					property_label = QString("Array<%1>").arg(property_label);
				}
				
				property_grid->addWidget(new QLabel(property_label), current_row, 1);
				++current_row;
			}

			property_group_box->setLayout(property_grid);
			
			m_ui.propertiesMainVBox->insertWidget(m_ui.propertiesMainVBox->count() - 1, property_group_box);
			parent_type_uuid = parent_type_data->info.base_id;
		}
	}

	void DataSchemaDialog::initialize()
	{
		Core::ProjectManager& project_manager = m_application.get_project_manager();
		const Core::DataSchema& project_data_schema = project_manager.get_project_data_schema();

		QHash<QUuid, QTreeWidgetItem*> type_tree_item_lookup;
		QTreeWidgetItem* base_types_root = new QTreeWidgetItem(m_ui.schemaTree);
		base_types_root->setText(0, "Base Types");

		const VadonEditor::Core::DataSchema::TypeMetadataRegistry& registry = project_data_schema.get_registry();
		for (size_t type_index = 0; type_index < registry.get_registered_type_count(); ++type_index)
		{
			const ::Vadon::Foundation::UUID type_uuid = registry.get_type_uuid(type_index);
			const QUuid type_qt_uuid = VadonEditor::Utilities::vadon_uuid_to_qt_uuid(type_uuid);

			if (type_tree_item_lookup.find(type_qt_uuid) != type_tree_item_lookup.end())
			{
				// Node already added
				continue;
			}

			const VadonEditor::Core::TypeData* type_data = project_data_schema.find_type_data(type_uuid);

			if (VadonEditor::Core::DataSchema::get_base_type(type_uuid) != ::Vadon::Foundation::BaseType::INVALID)
			{
				QTreeWidgetItem* base_type_node = new QTreeWidgetItem(base_types_root);
				base_type_node->setText(0, type_data->get_name());
				base_type_node->setData(0, c_tree_uuid_role, type_qt_uuid);
				continue;
			}

			QTreeWidgetItem* new_type_node;

			if (type_data->info.base_id.is_valid() == true)
			{
				// Has parent type, find or create node and add to it
				const QUuid base_qt_uuid = VadonEditor::Utilities::vadon_uuid_to_qt_uuid(type_data->info.base_id);
				auto tree_item_it = type_tree_item_lookup.find(base_qt_uuid);
				if (tree_item_it != type_tree_item_lookup.end())
				{
					new_type_node = new QTreeWidgetItem(tree_item_it.value());
				}
				else
				{
					// Parent type was not added yet, need to recursively add parent nodes
					QList<::Vadon::Foundation::UUID> parent_types;
					::Vadon::Foundation::UUID parent_id = type_data->info.base_id;
					while (true)
					{
						parent_types.push_back(parent_id);

						const VadonEditor::Core::TypeData* parent_type_data = project_data_schema.find_type_data(parent_id);
						if (parent_type_data->info.base_id.is_valid() == false)
						{
							// No more ancestors, add to root
							break;
						}
						else if (type_tree_item_lookup.find(VadonEditor::Utilities::vadon_uuid_to_qt_uuid(parent_type_data->info.base_id)) != type_tree_item_lookup.end())
						{
							// Ancestor already added
							break;
						}
						else
						{
							// Ancestor also needs to be added
							parent_id = parent_type_data->info.base_id;
						}
					}

					// For each ancestor, add to root or parent
					while (parent_types.isEmpty() == false)
					{
						const ::Vadon::Foundation::UUID ancestor_id = parent_types.back();
						const VadonEditor::Core::TypeData* ancestor_type_data = project_data_schema.find_type_data(ancestor_id);

						QTreeWidgetItem* ancestor_node;
						const QUuid ancestor_qt_uuid = VadonEditor::Utilities::vadon_uuid_to_qt_uuid(ancestor_id);
						if (ancestor_type_data->info.base_id.is_valid() == true)
						{
							auto ancestor_parent_it = type_tree_item_lookup.find(VadonEditor::Utilities::vadon_uuid_to_qt_uuid(ancestor_type_data->info.base_id));
							ancestor_node = new QTreeWidgetItem(ancestor_parent_it.value());
						}
						else
						{
							ancestor_node = new QTreeWidgetItem(m_ui.schemaTree);
						}
						ancestor_node->setText(0, ancestor_type_data->get_name());
						ancestor_node->setData(0, c_tree_uuid_role, ancestor_qt_uuid);
						type_tree_item_lookup.insert(ancestor_qt_uuid, ancestor_node);

						parent_types.pop_back();
					}

					// Finally add the node itself
					tree_item_it = type_tree_item_lookup.find(base_qt_uuid);
					Q_ASSERT_X(tree_item_it != type_tree_item_lookup.end(), "VadonEditor::UI::DataSchemaDialog::initialize", "Cannot find parent!");
					new_type_node = new QTreeWidgetItem(tree_item_it.value());
				}
			}
			else
			{
				// No base type, add as root node
				new_type_node = new QTreeWidgetItem(m_ui.schemaTree);
			}

			new_type_node->setText(0, type_data->get_name());
			new_type_node->setData(0, c_tree_uuid_role, type_qt_uuid);
			type_tree_item_lookup.insert(type_qt_uuid, new_type_node);
		}

		m_ui.schemaTree->setSortingEnabled(true);
		m_ui.schemaTree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	}
}