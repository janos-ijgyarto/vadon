#include <VadonEditor/UI/Project/ProjectSettingsDialog.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <QFileDialog>

namespace VadonEditor::UI
{
	ProjectSettingsDialog::ProjectSettingsDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_plugin_path_changed(false)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);

		const VadonEditor::Core::ProjectInfo& loaded_project_info = m_application.get_project_manager().get_project_info();
		
		m_ui.nameValueLabel->setText(loaded_project_info.name);
		m_ui.rootPathValueLabel->setText(loaded_project_info.root_path);
		if (loaded_project_info.plugin_path.isEmpty() == false)
		{
			m_ui.pluginPathLineEdit->setText(loaded_project_info.plugin_path);
		}
	}

	void ProjectSettingsDialog::accept()
	{
		if (m_plugin_path_changed == true)
		{
			m_application.get_project_manager().set_plugin_path(m_ui.pluginPathLineEdit->text());
		}

		QDialog::accept();
	}

	void ProjectSettingsDialog::plugin_path_changed(const QString& text)
	{
		const VadonEditor::Core::ProjectInfo& loaded_project_info = m_application.get_project_manager().get_project_info();
		m_plugin_path_changed = (loaded_project_info.plugin_path != text);
	}

	void ProjectSettingsDialog::plugin_path_browse_clicked()
	{
		// TODO: branch file type for other platforms!
		QString plugin_path = QFileDialog::getOpenFileName(this, "Select Plugin File", QDir::currentPath(), tr("Plugin Files (*.dll)"));
		if (plugin_path.isEmpty() == false)
		{
			m_ui.pluginPathLineEdit->setText(plugin_path);
		}
	}
}