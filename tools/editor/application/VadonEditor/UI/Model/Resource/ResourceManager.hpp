#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEMANAGER_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEMANAGER_HPP
#include <VadonEditor/UI/Project/Asset/AssetMessageBox.hpp>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class UnsavedResourceAssetFilter : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		UnsavedResourceAssetFilter(Core::Application& application, QObject* parent = nullptr);

		bool initialize();
	protected:
		bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
	private:
		Core::Application& m_application;
		QList<int> m_unsaved_resource_asset_ids;
	};

	class UnsavedResourceMessageBox : public AssetMessageBox
	{
		Q_OBJECT
	public:
		UnsavedResourceMessageBox(QAbstractItemModel* asset_model, QWidget* parent = nullptr);
	};

	class ResourceManager : public QObject
	{
		Q_OBJECT
	public:
	private slots:
		void asset_opened(const QModelIndex& index);
		void resource_widget_removed(QObject* widget_obj);
	private:
		ResourceManager(Core::Application& application);

		bool initialize();
		void shutdown();

		bool request_close();
		void force_close();

		void simulator_initialized();

		Core::Application& m_application;
		QHash<QUuid, QWidget*> m_resource_widgets;
		QHash<QWidget*, QUuid> m_widget_reverse_lookup;

		friend class UISystem;
	};
}
#endif