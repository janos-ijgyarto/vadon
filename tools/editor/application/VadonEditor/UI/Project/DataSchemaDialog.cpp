#include <VadonEditor/UI/Project/DataSchemaDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Data/Object.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/TypeInfo/Metadata.hpp>
#include <Vadon/Foundation/TypeInfo/Object.hpp>

#include <QGroupBox>
#include <QLabel>

namespace
{
	constexpr Qt::ItemDataRole c_tree_uuid_role = Qt::ItemDataRole::UserRole;

	QString get_property_type_label(const VadonEditor::Core::DataSchema& project_data_schema, const VadonEditor::Core::PropertyData& property_data, qsizetype type_list_offset = 0)
	{
		const QUuid type_uuid = property_data.type_list[type_list_offset];
		const ::Vadon::Foundation::Property::Category category = VadonEditor::Core::PropertyData::get_category(type_uuid);
		switch (category)
		{
		case ::Vadon::Foundation::Property::Category::TRIVIAL:
		{
			const VadonEditor::Core::TypeData* type_data = project_data_schema.find_type_data(type_uuid);
			return type_data->get_name();
		}
		case ::Vadon::Foundation::Property::Category::ARRAY:
		{
			const QString nested_label = get_property_type_label(project_data_schema, property_data, type_list_offset + 1);
			return QString("Array<%1>").arg(nested_label);
		}
		case ::Vadon::Foundation::Property::Category::DICTIONARY:
		{
			// TODO!
			return "Dictionary";
		}
		case ::Vadon::Foundation::Property::Category::OBJECT:
		{
			// Check if we use ObjectWrapper, otherwise assume type is explicit
			QUuid object_type = property_data.type_list[type_list_offset];
			if (object_type == VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ObjectWrapperSchema::c_type_uuid))
			{
				// Check whether an explicit type is provided
				const qsizetype object_type_offset = type_list_offset + 1;
				if (object_type_offset >= property_data.type_list.size())
				{
					// No type specified, so it's a generic object
					return "Object";
				}
				else
				{
					// Use explicit type
					object_type = property_data.type_list[object_type_offset];
				}
			}

			const VadonEditor::Core::TypeData* object_type_data = project_data_schema.find_type_data(object_type);
			return object_type_data->get_name();
		}
		case ::Vadon::Foundation::Property::Category::RESOURCE:
		{
			const qsizetype resource_type_offset = type_list_offset + 1;
			Q_ASSERT_X(resource_type_offset < property_data.type_list.size(), "get_property_type_label", "Invalid property data");

			const QUuid resource_type_uuid = property_data.type_list[resource_type_offset];
			const VadonEditor::Core::TypeData* resource_type_data = project_data_schema.find_type_data(resource_type_uuid);

			return QString("ResourceID<%1>").arg(resource_type_data->get_name());
		}
		default:
			Q_UNREACHABLE();
		}
	}
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

	void DataSchemaDialog::tree_item_selected(const QModelIndex& index)
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

		if (index.isValid() == false)
		{
			return;
		}

		QVariant item_data = m_type_tree_model.data(index, static_cast<int>(Core::TypeTreeDataRole::TYPE_UUID));
		if (item_data.isValid() == false)
		{
			return;
		}

		const QUuid type_qt_uuid = item_data.toUuid();

		Core::ProjectManager& project_manager = m_application.get_project_manager();
		const Core::DataSchema& project_data_schema = project_manager.get_project_data_schema();

		const VadonEditor::Core::TypeData* type_data = project_data_schema.find_type_data(type_qt_uuid);
		QUuid parent_type_uuid = Utilities::vadon_uuid_to_qt_uuid(type_data->info.id);
		while (parent_type_uuid.isNull() == false)
		{
			QGroupBox* property_group_box = new QGroupBox();
			const VadonEditor::Core::TypeData* parent_type_data = project_data_schema.find_type_data(parent_type_uuid);

			property_group_box->setTitle(parent_type_data->get_name());

			QGridLayout* property_grid = new QGridLayout;

			int current_row = 0;
			for (const ::Vadon::Foundation::UUID& property_uuid : parent_type_data->property_list)
			{
				const VadonEditor::Core::PropertyData* property_data = parent_type_data->find_property_data(Utilities::vadon_uuid_to_qt_uuid(property_uuid));
				property_grid->addWidget(new QLabel(property_data->get_name()), current_row, 0);

				const QString property_label = get_property_type_label(project_data_schema, *property_data);
				property_grid->addWidget(new QLabel(property_label), current_row, 1);
				++current_row;
			}

			property_group_box->setLayout(property_grid);
			
			m_ui.propertiesMainVBox->insertWidget(m_ui.propertiesMainVBox->count() - 1, property_group_box);
			parent_type_uuid = Utilities::vadon_uuid_to_qt_uuid(parent_type_data->info.base_id);
		}
	}

	void DataSchemaDialog::initialize()
	{
		Core::ProjectManager& project_manager = m_application.get_project_manager();
		const Core::DataSchema& project_data_schema = project_manager.get_project_data_schema();

		// FIXME: this is a bit ugly
		m_type_tree_model.setSourceModel(const_cast<QStandardItemModel*>(&project_data_schema.get_qt_model()));

		m_ui.typeTreeView->setModel(&m_type_tree_model);
		m_ui.typeTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	}
}