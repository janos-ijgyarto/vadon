#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENEDIALOG_HPP
#include <VadonEditor/UI/Project/Asset/AssetDialog.hpp>
#include <VadonEditor/UI/Model/Scene/ui_InheritedSceneDialog.h>
#include <QUuid>
namespace VadonEditor::UI
{
	class NewSceneDialog : public SaveAssetDialog
	{
		Q_OBJECT
	public:
		NewSceneDialog(Core::Application& application, QWidget* parent = nullptr, const QModelIndex& root_asset = QModelIndex());
	protected:
		void internal_finalize_asset_save() override;
	};

	class InheritedSceneDialog : public QDialog
	{
		Q_OBJECT
	public:
		InheritedSceneDialog(Core::Application& application, const QModelIndex& base_scene = QModelIndex(), QWidget* parent = nullptr);

		void accept() override;
	private slots:
		void browse_base_scene_clicked();
		void base_scene_selected(const QUuid& scene_uuid);

		void browse_inherited_scene_clicked();
		void inherited_scene_path_selected(const QString& asset_path);
	private:
		void update_controls();

		Core::Application& m_application;
		Ui::CreateInheritedSceneDialog m_ui;

		QUuid m_base_scene;
		QString m_inherited_scene_path;
	};
}
#endif