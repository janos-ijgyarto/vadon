#include <VadonEditor/UI/Model/Property/Object.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/UI/Model/Object/ObjectDialog.hpp>
#include <VadonEditor/UI/Model/Object/ObjectEditor.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	PropertyObject::PropertyObject(const QUuid& id, const QVariantMap& data_map, Model::Resource* owner_resource, const QUuid& base_type, bool allow_subclass, QWidget* parent)
		: PropertyWidget(id, data_map, parent)
		, m_object(owner_resource->get_application())
		, m_owner_resource(owner_resource)
		, m_base_type(base_type)
		, m_allow_subclass(allow_subclass)
		, m_read_only(false)
	{
		m_ui.setupUi(this);

		if (is_nullable() == true)
		{
			// NOTE: if nullable, the map is expected to be the both type ID and properties
			if (m_object.import_data(data_map) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::PropertyObject::PropertyObject", "Failed to import object data!");
				return;
			}

			m_ui.objectToolButton->addAction(m_ui.actionNew);
			connect(m_ui.actionNew, &QAction::triggered, this, &PropertyObject::new_triggered);

			m_ui.objectToolButton->addAction(m_ui.actionClear);
			connect(m_ui.actionClear, &QAction::triggered, this, &PropertyObject::clear_triggered);
		}
		else
		{
			// NOTE: if non-nullable, it's an explicit type, so we just load properties
			if (m_object.init_type(base_type) == false)
			{
				Q_ASSERT_X(m_object.is_valid() == false, "VadonEditor::UI::PropertyObject::PropertyObject", "Invalid type for object!");
				return;
			}

			m_object.load_properties(data_map);
		}

		generate_property_widgets();
	}

	void PropertyObject::set_read_only(bool read_only)
	{
		m_read_only = read_only;

		m_ui.objectToolButton->setVisible((read_only == false) && is_nullable());
		m_ui.objectToolButton->setEnabled((read_only == false) && is_nullable());

		if (m_object.is_valid() == true)
		{
			for (int item_index = 0; item_index < m_ui.propertyGroupBox->layout()->count(); ++item_index)
			{
				QWidget* current_widget = m_ui.propertyGroupBox->layout()->itemAt(item_index)->widget();
				ObjectEditor* object_editor = qobject_cast<ObjectEditor*>(current_widget);
				if (object_editor != nullptr)
				{
					object_editor->set_read_only(read_only);
					return;
				}
				else
				{
					// TODO: error!
				}
			}
		}
	}

	void PropertyObject::object_property_value_changed(const QUuid& id)
	{
		Q_UNUSED(id);

		// FIXME: instead of always updating the whole object, we should use "targeted" property updates!
		QVariantMap object_data;
		if (is_nullable() == true)
		{
			if (m_object.is_valid() == true)
			{
				object_data = m_object.export_data();
			}
		}
		else
		{
			object_data = m_object.get_property_map();
		}

		internal_set_value(object_data);
	}

	void PropertyObject::new_triggered()
	{
		Q_ASSERT_X(is_nullable() == true, "VadonEditor::UI::PropertyObject::new_triggered", "Property must be nullable!");

		NewObjectDialog* new_object_dialog = new NewObjectDialog(m_object.get_application(), m_base_type, this);
		connect(new_object_dialog, &NewObjectDialog::object_type_selected, this, &PropertyObject::new_object_type_selected);

		new_object_dialog->open();
	}
	
	void PropertyObject::clear_triggered()
	{
		Q_ASSERT_X(is_nullable() == true, "VadonEditor::UI::PropertyObject::clear_triggered", "Property must be nullable!");

		m_object.clear_data();
		internal_set_value(QVariantMap());

		clear_property_widgets();
	}

	void PropertyObject::new_object_type_selected(const QUuid& object_type)
	{
		Q_ASSERT_X(is_nullable() == true, "VadonEditor::UI::PropertyObject::new_object_type_selected", "Property must be nullable!");

		if (Core::TypeData::is_base_type(object_type) == false)
		{
			QMessageBox::critical(this, "Object Property Error", "Object must not be base type!");
			return;
		}

		clear_triggered();

		if (m_object.default_initialize(object_type) == false)
		{
			QMessageBox::critical(this, "Object Property Error", "Failed to initialize object!");
			return;
		}

		generate_property_widgets();
	}

	void PropertyObject::generate_property_widgets()
	{
		if (m_object.is_valid() == true)
		{
			QVBoxLayout* vbox_layout = new QVBoxLayout();

			ObjectEditor* object_editor = new ObjectEditor(m_object, m_owner_resource, this);
			connect(object_editor, &ObjectEditor::object_property_edited, this, &PropertyObject::object_property_value_changed);

			vbox_layout->addWidget(object_editor);
			m_ui.propertyGroupBox->setLayout(vbox_layout);

			set_read_only(m_read_only);

			setMinimumSize(QSize(400, 300));

			update_type_label();
		}
		else
		{
			clear_property_widgets();
		}
	}

	void PropertyObject::clear_property_widgets()
	{
		QLayout* layout = m_ui.propertyGroupBox->layout();
		if (layout != nullptr)
		{
			QLayoutItem* current_item = nullptr;
			while (current_item = layout->takeAt(0))
			{
				if (current_item->widget())
				{
					delete current_item->widget();
				}
				delete current_item;
			}
			delete layout;
		}

		// Reset minimum size
		m_ui.propertyGroupBox->adjustSize();
		setMinimumSize(QSize(0, 0));
		adjustSize();

		update_type_label();
	}

	void PropertyObject::update_type_label()
	{
		const QUuid type_id = m_object.is_valid() == true ? m_object.get_type_id() : m_base_type;

		const Core::DataSchema& data_schema = m_owner_resource->get_application().get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(type_id);
		QString current_type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);
		if (current_type_name.isEmpty())
		{
			current_type_name = QString("Object type %1").arg(type_id.toString());
		}

		m_ui.typeLabel->setText(current_type_name);
	}

	bool PropertyObject::is_nullable()
	{
		return (Utilities::is_uuid_valid(m_base_type) == false) || ((Utilities::is_uuid_valid(m_base_type) == true) && (m_allow_subclass == true));
	}
}