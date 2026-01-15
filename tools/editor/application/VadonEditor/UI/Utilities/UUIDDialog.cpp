#include <VadonEditor/UI/Utilities/UUIDDialog.hpp>

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
		m_ui.uuidLineEdit->setText(new_uuid.toString(QUuid::StringFormat::WithoutBraces));
	}
}