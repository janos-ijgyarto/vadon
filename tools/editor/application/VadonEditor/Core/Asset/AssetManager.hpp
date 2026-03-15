#ifndef VADONEDITOR_CORE_ASSET_ASSETMANAGER_HPP
#define VADONEDITOR_CORE_ASSET_ASSETMANAGER_HPP
#include <VadonEditor/Core/Asset/Asset.hpp>
#include <QStandardItemModel>
class QFileInfo;
namespace VadonEditor::Core
{
	class Application;

	class AssetManager : public QObject
	{
		Q_OBJECT
	public:
		QAbstractItemModel& get_model() { return m_asset_model; }

		QModelIndex create_asset(const AssetInfo& info);
		
		AssetInfo get_asset_info(const QModelIndex& index) const;

		bool save_asset_data(const QUuid& asset_id, QByteArrayView data);
		bool load_asset_data(const QUuid& asset_id, QByteArray& data) const;

		QModelIndex find_asset_index_by_path(const QString& path) const;
		void open_asset(const QModelIndex& index);
	signals:
		void asset_opened(const QModelIndex& index);
	private:
		struct InternalAssetInfo
		{
			AssetType type;
			QUuid file_id;
		};

		AssetManager(Application& application);

		bool initialize();

		void project_loaded();
		void scan_assets_recursive(const QFileInfo& dir_info, const QHash<QString, AssetType>& asset_type_lookup);

		QStandardItem* add_asset(const QString& path, const InternalAssetInfo& info);

		QStandardItem* find_asset_by_path(const QString& path) const;
		QStandardItem* find_asset_by_id(const QUuid& id) const;

		QString get_asset_path(QStandardItem* asset_item) const;

		QString get_asset_absolute_file_path(const QString& asset_path) const;
		QString get_asset_relative_path(const QString& asset_path) const;

		Application& m_application;

		// TODO: use a QFileSystemModel to watch for file changes?
		QStandardItemModel m_asset_model;

		QHash<QUuid, InternalAssetInfo> m_asset_lookup;

		friend Application;
	};
}
#endif