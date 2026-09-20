#ifndef VADONEDITOR_UI_MODEL_PROPERTY_OBJECT_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_OBJECT_HPP
#include <VadonEditor/Core/Data/Object.hpp>
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyObject.h>
namespace VadonEditor::UI
{
	class PropertyObject : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyObject(const QUuid& id, const QVariantMap& data_map, Model::Resource* owner_resource, const QUuid& base_type, bool allow_subclass, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void object_property_value_changed(const QUuid& id);

		void new_triggered();
		void clear_triggered();

		void new_object_type_selected(const QUuid& object_type);
	private:
		void generate_property_widgets();
		void clear_property_widgets();
		void update_type_label();

		bool is_nullable();

		Ui::PropertyObject m_ui;

		Core::DataObject m_object;
		Model::Resource* m_owner_resource;
		QUuid m_base_type;
		bool m_allow_subclass;
		bool m_read_only;
	};
}
#endif