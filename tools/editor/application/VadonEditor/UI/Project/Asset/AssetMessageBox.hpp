#ifndef VADONEDITOR_UI_PROJECT_ASSET_ASSETMESSAGEBOX_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_ASSETMESSAGEBOX_HPP
#include <VadonEditor/UI/Utilities/CustomMessageBox.hpp>
class QAbstractItemModel;
namespace VadonEditor::UI
{
	class AssetMessageBox : public CustomMessageBox
	{
		Q_OBJECT
	public:
		AssetMessageBox(QAbstractItemModel* asset_model, QWidget* parent = nullptr);
	protected slots:
		void dialog_button_clicked(QAbstractButton* button);
	};
}
#endif