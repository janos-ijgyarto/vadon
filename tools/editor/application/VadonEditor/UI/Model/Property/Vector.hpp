#ifndef VADONEDITOR_UI_MODEL_PROPERTY_VECTOR_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_VECTOR_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyVector2.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyVector2i.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyVector3.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyVector3i.h>
#include <VadonEditor/UI/Model/Property/ui_PropertyVector4.h>
namespace VadonEditor::UI
{
	class PropertyVector2 : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyVector2(const QUuid& id, const QVector2D& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed_x(double value);
		void value_changed_y(double value);
	private:
		void vector_value_changed();

		Ui::PropertyVector2 m_ui;
	};

	class PropertyVector2i : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyVector2i(const QUuid& id, const QPoint& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed_x(int value);
		void value_changed_y(int value);
	private:
		void vector_value_changed();

		Ui::PropertyVector2i m_ui;
	};

	class PropertyVector3 : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyVector3(const QUuid& id, const QVector3D& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed_x(double value);
		void value_changed_y(double value);
		void value_changed_z(double value);
	private:
		void vector_value_changed();

		Ui::PropertyVector3 m_ui;
	};

	class PropertyVector3i : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyVector3i(const QUuid& id, const QLine& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed_x(int value);
		void value_changed_y(int value);
		void value_changed_z(int value);
	private:
		void vector_value_changed();

		Ui::PropertyVector3i m_ui;
	};

	class PropertyVector4 : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyVector4(const QUuid& id, const QVector4D& value, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void value_changed_x(double value);
		void value_changed_y(double value);
		void value_changed_z(double value);
		void value_changed_w(double value);
	private:
		void vector_value_changed();

		Ui::PropertyVector4 m_ui;
	};
}
#endif