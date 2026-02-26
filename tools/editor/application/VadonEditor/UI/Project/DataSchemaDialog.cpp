#include <VadonEditor/UI/Project/DataSchemaDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/TypeInfo/Metadata.hpp>

#include <QGroupBox>
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

		// FIXME: this is a bit ugly
		m_type_tree_model.setSourceModel(const_cast<QStandardItemModel*>(&project_data_schema.get_qt_model()));

		m_ui.typeTreeView->setModel(&m_type_tree_model);
		m_ui.typeTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	}
}