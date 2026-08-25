#include <VadonEditor/UI/Project/Asset/AssetDialog.hpp>

namespace VadonEditor::UI
{
	SaveAssetDialog::SaveAssetDialog(Core::Application& application, QWidget* parent, const QModelIndex& root_asset)
		: QDialog(parent)
		, m_application(application)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.assetBrowser->initialize(application, root_asset);
	}

	QString SaveAssetDialog::get_asset_path() const
	{
		const QString root_path = m_ui.assetBrowser->get_current_root_path();
		return root_path.isEmpty() ? m_ui.assetBrowser->get_file_name() : QString("%1/%2").arg(root_path).arg(m_ui.assetBrowser->get_file_name());
	}

	void SaveAssetDialog::internal_finalize_asset_save()
	{
		emit(asset_saved(get_asset_path()));
		accept();
	}

	void SaveAssetDialog::asset_name_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();
	}

	void SaveAssetDialog::update_controls()
	{
		QPushButton* save_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Save);
		save_button->setEnabled(m_ui.assetBrowser->get_file_name().isEmpty() == false);
	}

	OpenAssetDialog::OpenAssetDialog(Core::Application& application, QWidget* parent)
		: QDialog(parent)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.assetBrowser->initialize(application);
	}

	void OpenAssetDialog::asset_name_changed(const QString& text)
	{
		Q_UNUSED(text);
		update_controls();
	}

	void OpenAssetDialog::finalize_asset_open()
	{
		const QString root_path = m_ui.assetBrowser->get_current_root_path();
		QString asset_path = root_path.isEmpty() ? m_ui.assetBrowser->get_file_name() : QString("%1/%2").arg(root_path).arg(m_ui.assetBrowser->get_file_name());
		emit(asset_opened(asset_path));

		accept();
	}

	void OpenAssetDialog::update_controls()
	{
		QPushButton* open_button = m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Open);
		open_button->setEnabled(m_ui.assetBrowser->get_file_name().isEmpty() == false);
	}
}