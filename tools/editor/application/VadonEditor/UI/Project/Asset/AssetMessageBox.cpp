#include <VadonEditor/UI/Project/Asset/AssetMessageBox.hpp>

#include <QPushButton>
#include <QTreeView>

namespace VadonEditor::UI
{
	AssetMessageBox::AssetMessageBox(QAbstractItemModel* asset_model, QWidget* parent)
		: CustomMessageBox(parent)
	{
		// Take ownership of model
		asset_model->setParent(this);

		QTreeView* asset_tree_view = new QTreeView();
		asset_tree_view->setModel(asset_model);
		asset_tree_view->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
		asset_tree_view->setDragEnabled(false);
		asset_tree_view->setHeaderHidden(true);
		asset_tree_view->setItemsExpandable(false);

		m_ui.mainLayout->insertWidget(0, asset_tree_view);

		asset_tree_view->expandAll();

		m_ui.buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Yes | QDialogButtonBox::StandardButton::No | QDialogButtonBox::StandardButton::Cancel);

		connect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(m_ui.buttonBox, &QDialogButtonBox::clicked, this, &AssetMessageBox::dialog_button_clicked);
	}

	void AssetMessageBox::dialog_button_clicked(QAbstractButton* button)
	{
		if (button == m_ui.buttonBox->button(QDialogButtonBox::StandardButton::No))
		{
			done(QDialog::DialogCode::Accepted + 1);
		}
		else if (button == m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Cancel))
		{
			reject();
		}
	}
}