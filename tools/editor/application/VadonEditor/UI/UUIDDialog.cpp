#include <VadonEditor/UI/UUIDDialog.hpp>

#include <QUuid>

namespace VadonEditor::UI
{
	UUIDDialog::UUIDDialog(QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		QObject::connect(m_ui.generateButton, &QPushButton::clicked, this, &UUIDDialog::generate_clicked);
	}

	void UUIDDialog::generate_clicked()
	{
		QUuid new_uuid = QUuid::createUuid();
		QUuid::Id128Bytes new_uuid_bytes = new_uuid.toBytes();

		QByteArray uuid_byte_array;
		uuid_byte_array.append(QByteArrayView(new_uuid_bytes));

		m_ui.uuidLineEdit->setText(uuid_byte_array.toBase64(QByteArray::Base64Encoding));
	}
}