#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <VadonEditor/Core/Project/DataSchema.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/Resource.hpp>

#include <QPushButton>

namespace VadonEditor::UI
{
	NewResourceDialog::NewResourceDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_type_filter_model(application)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		m_type_filter_model.set_root_type(QUuid::fromString(::Vadon::Foundation::ResourceSchema::c_type_uuid.string));

		validate_state();
	}

	void NewResourceDialog::type_selected(const QModelIndex& index)
	{
		internal_type_selected(index);
		validate_state();
	}

	void NewResourceDialog::type_double_clicked(const QModelIndex& index)
	{
		internal_type_selected(index);

		if (m_selected_type.isNull() == false)
		{
			accept();
		}
	}

	void NewResourceDialog::validate_state()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		ok_button->setEnabled(m_selected_type.isNull() == false);
	}

	void NewResourceDialog::internal_type_selected(const QModelIndex& index)
	{
		if (index.isValid() == true)
		{
			m_selected_type = m_type_filter_model.data(index, static_cast<Qt::ItemDataRole>(Core::TypeTreeDataRole::TYPE_UUID)).toUuid();
		}
		else
		{
			m_selected_type = QUuid();
		}
	}
}