#ifndef VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#define VADONEDITOR_UI_MODEL_SCENE_SCENEMANAGER_HPP
#include <VadonEditor/UI/Project/Asset/AssetMessageBox.hpp>
#include <QSortFilterProxyModel>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class UnsavedSceneAssetFilter : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		UnsavedSceneAssetFilter(Core::Application& application, QObject* parent = nullptr);

		bool initialize();
	protected:
		bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
	private:
		Core::Application& m_application;
		QList<int> m_unsaved_scene_asset_ids;
	};

	class UnsavedSceneMessageBox : public AssetMessageBox
	{
		Q_OBJECT
	public:
		UnsavedSceneMessageBox(QAbstractItemModel* asset_model, QWidget* parent = nullptr);
	};

	class SceneTree;
	class SceneManager : public QObject
	{
		Q_OBJECT
	public:
	private slots:
		void asset_opened(const QModelIndex& index);
		void scene_modified(const QUuid& scene_id);
		void scene_saved(const QUuid& scene_id);

		void current_scene_changed(int tab_index);
	private:
		SceneManager(Core::Application& application);

		bool initialize();
		void shutdown();

		bool request_close();
		void force_close();

		SceneTree* find_scene_tab(const QUuid& scene_id) const;
		void update_scene_tab_label(SceneTree* scene_tree) const;

		Core::Application& m_application;

		friend class UISystem;
	};
}
#endif