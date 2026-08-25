#ifndef VADONEDITOR_UI_MODEL_PROPERTY_PROPERTY_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_PROPERTY_HPP
#include <VadonEditor/Core/Data/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyListEntry.h>
#include <QUuid>
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::UI
{
	struct PropertyWidgetInfo
	{
		QUuid property_id;
		QList<QUuid> type_list;
		qsizetype type_list_offset = 0;
		QVariant init_value;
	};

	class PropertyWidget : public QWidget
	{
		Q_OBJECT
	public:
		const QUuid& get_id() const { return m_id; }
		const QVariant& get_value() const { return m_value; }

		// TODO: add a "metadata" parameter
		// DataSchema will contain user-provided metadata to adjust editor UI (e.g range for numeric property)
		// and we'll also need it for other contexts!
		static PropertyWidget* create_widget(const PropertyWidgetInfo& info, QWidget* parent_widget, Model::Resource* owner_resource);

		virtual void set_read_only(bool read_only) = 0;
	signals:
		void value_changed(QUuid id);
	protected:
		PropertyWidget(const QUuid& id, const QVariant& init_value, QWidget* parent)
			: QWidget(parent)
			, m_id(id)
			, m_value(init_value)
		{}

		void internal_set_value(QVariant value)
		{
			m_value = value;
			emit(value_changed(m_id));
		}
	private:
		QUuid m_id;
		QVariant m_value;
	};

	class PropertyListEntry : public QWidget
	{
		Q_OBJECT
	public:
		PropertyListEntry(QWidget* parent, PropertyWidget* property_widget, const QString& label);

		PropertyWidget* get_property_widget() const;
	private:
		Ui::PropertyListEntry m_ui;
	};
}
#endif