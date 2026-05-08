#include <VadonEditor/UI/Model/Property/Property.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Data/Type.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Model/Property/CheckBox.hpp>
#include <VadonEditor/UI/Model/Property/Color.hpp>
#include <VadonEditor/UI/Model/Property/Numeric.hpp>
#include <VadonEditor/UI/Model/Property/Resource.hpp>
#include <VadonEditor/UI/Model/Property/Text.hpp>
#include <VadonEditor/UI/Model/Property/Vector.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::UI
{
	PropertyWidget* PropertyWidget::create_widget(const PropertyWidgetInfo& info, QWidget* parent_widget, Model::Resource* owner_resource)
	{
		switch (info.category)
		{
		case Core::PropertyCategory::TRIVIAL:
		{
			const ::Vadon::Foundation::BaseType base_type = Core::TypeData::get_base_type(info.data_type);
			switch (base_type)
			{
			case ::Vadon::Foundation::BaseType::INT32:
				return new PropertySpinBox(info.property_id, info.init_value.toInt(), parent_widget);
			case ::Vadon::Foundation::BaseType::UINT32:
				return new PropertySpinBox(info.property_id, info.init_value.toUInt(), parent_widget);
			case ::Vadon::Foundation::BaseType::FLOAT:
				return new PropertyDoubleSpinBox(info.property_id, info.init_value.toDouble(), parent_widget);
			case ::Vadon::Foundation::BaseType::BOOL:
				return new PropertyCheckBox(info.property_id, info.init_value.toBool(), parent_widget);
			case ::Vadon::Foundation::BaseType::STRING:
			{
				// TODO: check metadata, see if we should make it a line edit!
				return new PropertyPlainTextEdit(info.property_id, info.init_value.toString(), parent_widget);
			}
			case ::Vadon::Foundation::BaseType::VECTOR2:
				return new PropertyVector2(info.property_id, info.init_value.value<QVector2D>(), parent_widget);
			case ::Vadon::Foundation::BaseType::VECTOR2I:
				return new PropertyVector2i(info.property_id, info.init_value.toPoint(), parent_widget);
			case ::Vadon::Foundation::BaseType::VECTOR3:
				return new PropertyVector3(info.property_id, info.init_value.value<QVector3D>(), parent_widget);
			case ::Vadon::Foundation::BaseType::VECTOR3I:
				return new PropertyVector3i(info.property_id, info.init_value.toLine(), parent_widget);
			case ::Vadon::Foundation::BaseType::VECTOR4:
				return new PropertyVector4(info.property_id, info.init_value.value<QVector4D>(), parent_widget);
			case ::Vadon::Foundation::BaseType::COLORRGBA:
				return new PropertyColorRGBA(info.property_id, info.init_value.value<QColor>(), parent_widget);
			case ::Vadon::Foundation::BaseType::UUID:
				// FIXME: create dedicated widget that can generate UUID!
				return new PropertyLineEdit(info.property_id, info.init_value.toUuid().toString(), parent_widget);
			}
		}
		break;
		case Core::PropertyCategory::RESOURCE:
		{
			Model::Resource* resource = nullptr;
			QUuid resource_id = info.init_value.toUuid();
			if (resource_id.isNull() == false)
			{
				Core::Application& application = owner_resource->get_application();
				resource = application.get_model_system().get_resource_system().get_resource(resource_id);
			}

			return new PropertyResource(info.property_id, resource, owner_resource, info.data_type, parent_widget);
		}
		case Core::PropertyCategory::ARRAY:
		{
			// TODO
			break;
		}
		}

		return nullptr;
	}

	PropertyListEntry::PropertyListEntry(QWidget* parent, PropertyWidget* property_widget, const QString& label)
		: QWidget(parent)
	{
		m_ui.setupUi(this);

		m_ui.label->setText(label);

		QHBoxLayout* hbox_layout = new QHBoxLayout();
		hbox_layout->addWidget(property_widget);

		m_ui.frame->setLayout(hbox_layout);
	}

	PropertyWidget* PropertyListEntry::get_property_widget() const
	{
		return qobject_cast<PropertyWidget*>(m_ui.frame->layout()->itemAt(0)->widget());
	}
}