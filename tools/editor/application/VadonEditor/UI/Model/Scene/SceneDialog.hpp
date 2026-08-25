#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENEDIALOG_HPP
#include <VadonEditor/UI/Project/Asset/AssetDialog.hpp>
namespace VadonEditor::UI
{
	class NewSceneDialog : public SaveAssetDialog
	{
		Q_OBJECT
	public:
		NewSceneDialog(Core::Application& application, QWidget* parent = nullptr, const QModelIndex& root_asset = QModelIndex());
	protected:
		void internal_finalize_asset_save() override;
	private:

	};
}
#endif