#include <VadonEditor/UI/Project/NewProjectDialog.hpp>

#include <VadonEditor/Core/Project/Project.hpp>

#include <QFileDialog>

namespace
{
	bool validate_folder_path(const QString& path)
	{
		if (path.isEmpty() == true)
		{
			return false;
		}

		return QFileInfo(path).isDir() == true;
	}
}

namespace VadonEditor::UI
{
	NewProjectDialog::NewProjectDialog(const Core::SourceProjectInfo& source_info, QWidget* parent)
		: QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		if (source_info.is_valid() == true)
		{
			// We already have the base info, so we're importing an existing project
			m_ui.nameLineEdit->setText(source_info.name);
			m_ui.nameLineEdit->setReadOnly(true);

			m_ui.rootPathLineEdit->blockSignals(true);
			m_ui.rootPathLineEdit->setText(source_info.root_path);
			m_ui.rootPathLineEdit->setReadOnly(true);
			m_ui.rootPathLineEdit->blockSignals(false);
		}

		validate_state();
	}

	Core::SourceProjectInfo NewProjectDialog::get_source_info() const
	{
		Core::SourceProjectInfo source_info;

		source_info.name = m_ui.nameLineEdit->text();
		source_info.root_path = m_ui.rootPathLineEdit->text();

		return source_info;
	}

	void NewProjectDialog::name_edited(const QString& text)
	{
		Q_UNUSED(text);
		validate_state();
	}

	void NewProjectDialog::root_path_changed(const QString& text)
	{
		Q_UNUSED(text);
		validate_state();
	}

	void NewProjectDialog::root_path_browse_clicked()
	{
		QString project_path = QFileDialog::getExistingDirectory(this, "Select Project Path", QDir::currentPath(), QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		if (project_path.isEmpty() == false)
		{
			m_ui.rootPathLineEdit->setText(project_path);
		}
	}

	void NewProjectDialog::output_path_changed(const QString& text)
	{
		Q_UNUSED(text);
		validate_state();
	}

	void NewProjectDialog::output_path_browse_clicked()
	{
		QString start_dir = m_ui.rootPathLineEdit->text();
		if (start_dir.isEmpty() == true)
		{
			start_dir = QDir::currentPath();
		}

		QString output_path = QFileDialog::getExistingDirectory(this, "Select Output Path", start_dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (output_path.isEmpty() == false)
		{
			m_ui.outputPathLineEdit->setText(output_path);
		}
	}

	void NewProjectDialog::validate_state()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		
		const bool valid_name = m_ui.nameLineEdit->text().isEmpty() == false;
		const bool valid_path = validate_folder_path(m_ui.rootPathLineEdit->text());
		const bool valid_output_path = validate_folder_path(m_ui.outputPathLineEdit->text());

		ok_button->setEnabled(valid_name && valid_path && valid_output_path);
	}
}