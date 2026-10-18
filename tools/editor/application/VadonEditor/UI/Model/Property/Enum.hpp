#ifndef VADONEDITOR_UI_MODEL_PROPERTY_ENUM_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_ENUM_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyEnum.h>
namespace VadonEditor::UI
{
	class PropertyEnum : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyEnum(Core::Application& application, const QUuid& id, const QUuid& enum_type_id, const QUuid& init_value, QWidget* parent = nullptr);

		void set_read_only(bool read_only) override;
		void set_enum_value(const QUuid& value);
	private slots:
		void enum_value_selected(int index);
	private:
		Ui::PropertyEnum m_ui;

		Core::Application& m_application;
		const QUuid m_enum_type_id;
	};
}
#endif