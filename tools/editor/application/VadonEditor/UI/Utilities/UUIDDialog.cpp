#include <VadonEditor/UI/Utilities/UUIDDialog.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::UI
{
	UUIDDialog::UUIDDialog(QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);
	}

	void UUIDDialog::generate_clicked()
	{
		QUuid new_uuid = QUuid::createUuid();
		set_uuid_line_edit(new_uuid);
		set_base64_line_edit(new_uuid);
	}

	void UUIDDialog::uuid_edited(const QString& text)
	{
		const QUuid parsed_uuid = QUuid::fromString(text);
		set_base64_line_edit(parsed_uuid);
	}

	void UUIDDialog::base64_edited(const QString& text)
	{
		const QUuid parsed_uuid = Utilities::base64_string_to_uuid(text);
		set_uuid_line_edit(parsed_uuid);
	}

	void UUIDDialog::set_uuid_line_edit(const QUuid& uuid_value)
	{
		if (Utilities::is_uuid_valid(uuid_value) == true)
		{
			m_ui.uuidLineEdit->setText(uuid_value.toString(QUuid::StringFormat::WithoutBraces));
		}
		else
		{
			m_ui.uuidLineEdit->setText("INVALID UUID");
		}
	}

	void UUIDDialog::set_base64_line_edit(const QUuid& uuid_value)
	{
		if (Utilities::is_uuid_valid(uuid_value) == true)
		{
			m_ui.base64LineEdit->setText(Utilities::uuid_to_base64_string(uuid_value));
		}
		else
		{
			m_ui.base64LineEdit->setText("INVALID UUID");
		}
	}
}