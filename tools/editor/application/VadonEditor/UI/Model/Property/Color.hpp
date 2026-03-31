#ifndef VADONEDITOR_UI_MODEL_PROPERTY_COLOR_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_COLOR_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyColorRGBA.h>
namespace VadonEditor::UI
{
	class PropertyColorRGBA : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyColorRGBA(const QUuid& id, const QColor& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void color_picker_button_clicked();
	private:
		void internal_set_color(const QColor& value);

		Ui::PropertyColorRGBA m_ui;
	};
}
#endif