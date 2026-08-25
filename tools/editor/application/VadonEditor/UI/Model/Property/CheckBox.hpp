#ifndef VADONEDITOR_UI_MODEL_PROPERTY_CHECKBOX_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_CHECKBOX_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyCheckBox.h>
namespace VadonEditor::UI
{
	class PropertyCheckBox : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyCheckBox(const QUuid& id, bool value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void check_state_changed(Qt::CheckState state);
	private:
		Ui::PropertyCheckBox m_ui;
	};
}
#endif