#ifndef VADONEDITOR_UI_MODEL_PROPERTY_NUMERIC_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_NUMERIC_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertySpinBox.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyDoubleSpinBox.h>
namespace VadonEditor::UI
{
	class PropertySpinBox : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertySpinBox(const QUuid& id, int value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed(int value);
	private:
		Ui::PropertySpinBox m_ui;
	};

	class PropertyDoubleSpinBox : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyDoubleSpinBox(const QUuid& id, double value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed(double value);
	private:
		Ui::PropertyDoubleSpinBox m_ui;
	};
}
#endif