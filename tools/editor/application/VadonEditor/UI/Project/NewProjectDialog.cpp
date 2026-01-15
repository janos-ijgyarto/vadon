#include <VadonEditor/UI/Project/NewProjectDialog.hpp>

#include <VadonEditor/Core/Project/Project.hpp>

#include <QFileDialog>

namespace VadonEditor::UI
{
	NewProjectDialog::NewProjectDialog(QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		validate_state();
	}

	VadonEditor::Core::ProjectInfo NewProjectDialog::get_project_info() const
	{
		VadonEditor::Core::ProjectInfo project_info;

		project_info.name = m_ui.nameLineEdit->text();
		project_info.root_path = m_ui.rootPathLineEdit->text();

		return project_info;
	}

	void NewProjectDialog::name_edited(const QString& text)
	{
		Q_UNUSED(text);
		validate_state();
	}

	void NewProjectDialog::root_path_edited(const QString& text)
	{
		Q_UNUSED(text);
		validate_state();
	}

	void NewProjectDialog::browse_clicked()
	{
		QString project_path = QFileDialog::getExistingDirectory(this, "Select Project Path", QDir::currentPath(), QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		if (project_path.isEmpty() == false)
		{
			m_ui.rootPathLineEdit->setText(project_path);
		}
	}

	void NewProjectDialog::validate_state()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		
		const bool valid_name = m_ui.nameLineEdit->text().isEmpty() == false;
		const bool valid_path = m_ui.rootPathLineEdit->text().isEmpty() == false;

		ok_button->setEnabled(valid_name && valid_path);
	}
}