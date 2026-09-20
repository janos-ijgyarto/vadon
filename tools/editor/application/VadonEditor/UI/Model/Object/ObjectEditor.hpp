#ifndef VADONEDITOR_UI_MODEL_OBJECT_OBJECTEDITOR_HPP
#define VADONEDITOR_UI_MODEL_OBJECT_OBJECTEDITOR_HPP
#include <VadonEditor/UI/Model/Object/ui_ObjectEditor.h>
#include <QUuid>
namespace VadonEditor::Core
{
	class DataObject;
}
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::UI
{
	class PropertyWidget;

	class ObjectEditor : public QWidget
	{
		Q_OBJECT
	public:
		ObjectEditor(Core::DataObject& data_object, Model::Resource* owner_resource, QWidget* parent = nullptr);

		const Core::DataObject& get_object() const { return m_object; }

		void set_read_only(bool read_only);

		static QWidgetList generate_property_widgets(const Core::DataObject& data_object, Model::Resource* owner_resource, QWidget* parent_widget);
	signals:
		// FIXME: emit a "path" so the owning object can update the nested property in a targeted way (instead of updating the whole sub-object)
		void object_property_edited(const QUuid& property_id);
	private slots:
		void internal_property_edited(const QUuid& property_id);
	private:
		void initialize(Model::Resource* owner_resource);

		PropertyWidget* find_property_widget(const QUuid& property_id) const;

		Core::DataObject& m_object;
		Ui::ObjectEditor m_ui;
	};
}
#endif