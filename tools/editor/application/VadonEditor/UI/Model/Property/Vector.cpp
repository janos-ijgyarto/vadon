#include <VadonEditor/UI/Model/Property/Vector.hpp>

#include <QVector3D>

namespace
{
	void init_spinbox_min_max(QSpinBox* spin_box)
	{
		spin_box->setMinimum(INT_MIN);
		spin_box->setMaximum(INT_MAX);
	}

	void init_double_spinbox_min_max(QDoubleSpinBox* spin_box)
	{
		spin_box->setMinimum(-1 * std::numeric_limits<double>::max());
		spin_box->setMaximum(std::numeric_limits<double>::max());
	}
}

namespace VadonEditor::UI
{
	PropertyVector2::PropertyVector2(const QUuid& id, const QVector2D& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		init_double_spinbox_min_max(m_ui.xSpinBox);
		init_double_spinbox_min_max(m_ui.ySpinBox);

		m_ui.xSpinBox->setValue(value.x());
		m_ui.ySpinBox->setValue(value.y());
	}

	void PropertyVector2::set_read_only(bool read_only)
	{
		m_ui.xSpinBox->setReadOnly(read_only);
		m_ui.ySpinBox->setReadOnly(read_only);
	}

	void PropertyVector2::value_changed_x(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector2::value_changed_y(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector2::vector_value_changed()
	{
		internal_set_value(QVector2D(m_ui.xSpinBox->value(), m_ui.ySpinBox->value()));
	}

	PropertyVector2i::PropertyVector2i(const QUuid& id, const QPoint& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.xSpinBox->setValue(value.x());
		m_ui.ySpinBox->setValue(value.y());
	}

	void PropertyVector2i::set_read_only(bool read_only)
	{
		m_ui.xSpinBox->setReadOnly(read_only);
		m_ui.ySpinBox->setReadOnly(read_only);
	}

	void PropertyVector2i::value_changed_x(int value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector2i::value_changed_y(int value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector2i::vector_value_changed()
	{
		internal_set_value(QPoint(m_ui.xSpinBox->value(), m_ui.ySpinBox->value()));
	}
		
	PropertyVector3::PropertyVector3(const QUuid& id, const QVector3D& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.xSpinBox->setValue(value.x());
		m_ui.ySpinBox->setValue(value.y());
		m_ui.zSpinBox->setValue(value.z());
	}

	void PropertyVector3::set_read_only(bool read_only)
	{
		m_ui.xSpinBox->setReadOnly(read_only);
		m_ui.ySpinBox->setReadOnly(read_only);
		m_ui.zSpinBox->setReadOnly(read_only);
	}

	void PropertyVector3::value_changed_x(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3::value_changed_y(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3::value_changed_z(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3::vector_value_changed()
	{
		internal_set_value(QVector3D(m_ui.xSpinBox->value(), m_ui.ySpinBox->value(), m_ui.zSpinBox->value()));
	}

	PropertyVector3i::PropertyVector3i(const QUuid& id, const QLine& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.xSpinBox->setValue(value.x1());
		m_ui.ySpinBox->setValue(value.y1());
		m_ui.zSpinBox->setValue(value.x2());
	}

	void PropertyVector3i::set_read_only(bool read_only)
	{
		m_ui.xSpinBox->setReadOnly(read_only);
		m_ui.ySpinBox->setReadOnly(read_only);
		m_ui.zSpinBox->setReadOnly(read_only);
	}

	void PropertyVector3i::value_changed_x(int value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3i::value_changed_y(int value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3i::value_changed_z(int value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector3i::vector_value_changed()
	{
		internal_set_value(QLine(m_ui.xSpinBox->value(), m_ui.ySpinBox->value(), m_ui.zSpinBox->value(), 0));
	}

	PropertyVector4::PropertyVector4(const QUuid& id, const QVector4D& value, QWidget* parent)
		: PropertyWidget(id, value, parent)
	{
		m_ui.setupUi(this);

		m_ui.xSpinBox->setValue(value.x());
		m_ui.ySpinBox->setValue(value.y());
		m_ui.zSpinBox->setValue(value.z());
		m_ui.wSpinBox->setValue(value.w());
	}

	void PropertyVector4::set_read_only(bool read_only)
	{
		m_ui.xSpinBox->setReadOnly(read_only);
		m_ui.ySpinBox->setReadOnly(read_only);
		m_ui.zSpinBox->setReadOnly(read_only);
		m_ui.wSpinBox->setReadOnly(read_only);
	}

	void PropertyVector4::value_changed_x(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector4::value_changed_y(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector4::value_changed_z(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector4::value_changed_w(double value)
	{
		Q_UNUSED(value);
		vector_value_changed();
	}

	void PropertyVector4::vector_value_changed()
	{
		internal_set_value(QVector4D(m_ui.xSpinBox->value(), m_ui.ySpinBox->value(), m_ui.zSpinBox->value(), m_ui.wSpinBox->value()));
	}
}