#include <VadonEditor/UI/Project/Asset/AddFolderDialog.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QValidator>

namespace
{
	// TODO: implement less restrictive validator which checks with filesystem if name is valid
	constexpr const char* c_valid_filename_regex = R"(^[a-zA-Z0-9_-]*$)";
}

namespace VadonEditor::UI
{
	AddFolderDialog::AddFolderDialog(Core::Application& application, QModelIndex parent_asset, QWidget* parent)
		: QDialog(parent)
		, m_application(application)
		, m_parent_asset(parent_asset)
	{
		setAttribute(Qt::WA_DeleteOnClose, true);
		m_ui.setupUi(this);
			
		QString parent_path = ".";
		if (m_parent_asset.isValid() == true)
		{
			Core::AssetManager& asset_manager = m_application.get_asset_manager();
			const Core::AssetInfo parent_info = asset_manager.get_asset_info(m_parent_asset);

			parent_path = parent_info.path;
			if (parent_info.type != Core::AssetType::FOLDER)
			{
				parent_path = parent_info.get_parent_path();
			}
		}
		m_ui.nameLabel->setText(m_ui.nameLabel->text() + QString(" %1/").arg(parent_path));

		QRegularExpression valid_filename_regex(c_valid_filename_regex);
		QRegularExpressionValidator* validator = new QRegularExpressionValidator(valid_filename_regex, this);
		m_ui.nameLineEdit->setValidator(validator);

		update_controls();
	}

	void AddFolderDialog::folder_name_accepted()
	{
		const QString folder_name = m_ui.nameLineEdit->text();
		if (folder_name.isEmpty() == true)
		{
			QMessageBox::critical(this, "Add Folder Error", "Must give folder a valid name");
			return;
		}

		Core::AssetManager& asset_manager = m_application.get_asset_manager();

		QString path;
		if (m_parent_asset.isValid() == true)
		{
			const Core::AssetInfo parent_info = asset_manager.get_asset_info(m_parent_asset);
			QString parent_path = parent_info.path;
			if (parent_info.type != Core::AssetType::FOLDER)
			{
				parent_path = parent_info.get_parent_path();
			}
			path = QDir::cleanPath(parent_path + Core::AssetInfo::c_dir_separator + folder_name);
		}
		else 
		{
			path = folder_name;
		}
		
		QModelIndex existing_asset_index = asset_manager.find_asset_index_by_path(path);
		if (existing_asset_index.isValid())
		{
			QMessageBox::critical(this, "Add Folder Error", "Folder already exists!");
			return;
		}

		Core::AssetInfo folder_info;
		folder_info.path = path;
		folder_info.type = Core::AssetType::FOLDER;

		const QModelIndex new_folder_index = asset_manager.create_asset(folder_info);
		if (new_folder_index.isValid() == false)
		{
			QMessageBox::critical(this, "Add Folder Error", "Failed to create folder!");
			return;
		}

		accept();
	}

	void AddFolderDialog::folder_name_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();
	}

	void AddFolderDialog::update_controls()
	{
		QPushButton* ok_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		ok_button->setEnabled(m_ui.nameLineEdit->text().isEmpty() == false);
	}
}