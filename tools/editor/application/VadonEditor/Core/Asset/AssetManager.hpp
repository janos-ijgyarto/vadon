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
		QStandardItemModel& get_model() { return m_asset_model; }

		QModelIndex create_asset(const AssetInfo& info);
		QModelIndex import_asset_file(const QString& file_path);
		
		AssetInfo get_asset_info(const QModelIndex& index) const;
		void set_asset_modified(const QModelIndex& index, bool modified);

		bool save_asset_data(int asset_id, QByteArrayView data);
		bool load_asset_data(int asset_id, QByteArray& data) const;

		bool save_temp_file_data(QStringView temp_file_relative_path, QByteArrayView data);

		bool load_imported_file_data(int asset_id, QByteArray& data) const;

		QModelIndex find_asset_index(int asset_id) const;
		QModelIndex find_asset_index_by_path(const QString& path) const;
		void open_asset(const QModelIndex& index);
	signals:
		void asset_opened(const QModelIndex& index);
	private:
		struct InternalAssetInfo
		{
			int id = AssetInfo::c_invalid_file_id;
			AssetType type;
		};

		AssetManager(Application& application);

		bool initialize();

		void project_loaded();
		void scan_assets_recursive(const QFileInfo& dir_info, const QHash<QString, AssetType>& asset_type_lookup);

		QStandardItem* add_asset(const QString& path, const InternalAssetInfo& info);

		QStandardItem* find_asset_by_path(const QString& path) const;
		QStandardItem* find_asset_by_id(int id) const;

		QString get_asset_path(const QStandardItem* asset_item) const;

		QString get_asset_absolute_file_path(const QString& asset_path) const;
		QString get_asset_relative_path(const QString& asset_path) const;

		int generate_new_asset_id();

		Application& m_application;

		// TODO: use a QFileSystemModel to watch for file changes?
		QStandardItemModel m_asset_model;
		int m_id_counter; // FIXME: this may be unnecessary if we can trust that QStandardItems are stable in memory

		QHash<QUuid, InternalAssetInfo> m_asset_lookup;

		friend Application;
	};
}
#endif