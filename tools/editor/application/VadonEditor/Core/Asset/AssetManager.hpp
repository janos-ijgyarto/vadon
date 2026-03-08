#ifndef VADONEDITOR_CORE_ASSET_ASSETMANAGER_HPP
#define VADONEDITOR_CORE_ASSET_ASSETMANAGER_HPP
#include <VadonEditor/Core/Asset/Asset.hpp>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
namespace VadonEditor::Core
{
	class Application;

	class AssetFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		struct InternalAssetInfo
		{
			AssetType type = AssetType::NONE;
			QUuid file_id;
		};

		void add_asset(const QString& path, const InternalAssetInfo& info);
		void remove_asset(const QString& path);

		InternalAssetInfo get_internal_asset_info(const QModelIndex& index) const;
		AssetInfo get_asset_info(const QModelIndex& index) const;

		QString find_asset_file(const QUuid& file_id) const;
	protected:
		// TODO: custom icons for scenes/resources/etc.
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
	private:
		AssetFilterModel();

		QHash<QString, InternalAssetInfo> m_asset_lookup;
		QHash<QUuid, QString> m_file_lookup;

		friend class AssetManager;
	};

	class AssetManager : public QObject
	{
		Q_OBJECT
	public:
		QAbstractItemModel& get_model() { return m_asset_filter_model; }

		bool create_asset(const AssetInfo& info);
		
		AssetInfo get_asset_info(const QModelIndex& index) const;

		bool save_asset_data(const QUuid& asset_id, QByteArrayView data);
		bool load_asset_data(const QUuid& asset_id, QByteArray& data) const;

		void open_asset(const QModelIndex& index);
	signals:
		void asset_opened(const QModelIndex& index);
	private:
		AssetManager(Application& application);

		bool initialize();

		bool project_loaded();
		void directory_loaded(const QString& path);

		Application& m_application;

		QFileSystemModel m_file_model;
		AssetFilterModel m_asset_filter_model;

		friend Application;
	};
}
#endif