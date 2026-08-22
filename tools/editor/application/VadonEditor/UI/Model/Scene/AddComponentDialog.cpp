#include <VadonEditor/UI/Model/Scene/AddComponentDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Scene/Entity.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <QPushButton>
#include <QUuid>

namespace
{
	static constexpr int c_component_type_role = Qt::ItemDataRole::UserRole + 1;
}

namespace VadonEditor::UI
{
	AddComponentDialog::AddComponentDialog(Core::Application& application, const QList<QUuid>& existing_components, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_existing_components(existing_components)
	{
		m_ui.setupUi(this);

		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		initialize();

		component_selection_changed();
	}

	void AddComponentDialog::accept()
	{
		if (validate_component_selection() == true)
		{
			QDialog::accept();
		}
	}

	void AddComponentDialog::filter_text_changed(const QString& text)
	{
		// Clear any prior selection since we are changing the filter
		m_ui.componentList->clearSelection();

		for (int list_index = 0; list_index < m_ui.componentList->count(); ++list_index)
		{
			QListWidgetItem* current_item = m_ui.componentList->item(list_index);
			if (text.isEmpty() == true)
			{
				current_item->setHidden(false);
			}
			else
			{
				current_item->setHidden(current_item->text().contains(text, Qt::CaseSensitivity::CaseInsensitive) == false);
			}
		}
	}

	void AddComponentDialog::component_selection_changed()
	{
		QList<QListWidgetItem*> selected_items = m_ui.componentList->selectedItems();
		if (selected_items.isEmpty() == true)
		{
			m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
		}
		else
		{
			m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
		}
	}

	void AddComponentDialog::initialize()
	{
		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();
		const Core::DataSchema::TypeMetadataRegistry& type_registry = data_schema.get_registry();

		// Go over all types, select the ones that have the "COMPONENT" metadata
		// FIXME: implement a proper filter model which can filter all these types in the registry!
		const size_t type_count = type_registry.get_registered_type_count();
		for (size_t type_index = 0; type_index < type_count; ++type_index)
		{
			const ::Vadon::Foundation::UUID type_uuid = type_registry.get_type_uuid(type_index);
			const QUuid type_qt_uuid = Utilities::vadon_uuid_to_qt_uuid(type_uuid);
			const Core::TypeData* type_data = data_schema.find_type_data(type_qt_uuid);

			const QString component_metadata = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::COMPONENT);
			if (component_metadata.isEmpty() == false)
			{
				if (m_existing_components.indexOf(type_qt_uuid) != -1)
				{
					// Component already added to entity
					continue;
				}

				// Component not present in entity, so we can add it to the list
				QString component_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
				if (component_name.isEmpty() == true)
				{
					component_name = QString("Component %1").arg(Utilities::uuid_to_base64_string(type_qt_uuid));
				}

				QListWidgetItem* component_item = new QListWidgetItem(component_name, m_ui.componentList);
				component_item->setData(c_component_type_role, type_qt_uuid);
			}
		}

		m_ui.componentList->sortItems();
	}

	bool AddComponentDialog::validate_component_selection()
	{
		QList<QListWidgetItem*> selected_items = m_ui.componentList->selectedItems();
		if (selected_items.isEmpty() == true)
		{
			Q_ASSERT_X(false, "AddComponentDialog::finalize_component_selection", "No selected items!");
			return false;
		}

		QListWidgetItem* selected_component_item = selected_items.first();
		const QUuid component_type_id = selected_component_item->data(c_component_type_role).toUuid();
		if (Utilities::is_uuid_valid(component_type_id) == false)
		{
			Q_ASSERT_X(false, "AddComponentDialog::finalize_component_selection", "Component UUID invalid");
			return false;
		}

		emit(component_type_selected(component_type_id));
		return true;
	}
}