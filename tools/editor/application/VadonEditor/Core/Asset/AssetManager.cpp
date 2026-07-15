#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Asset/Asset.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <QApplication>
#include <QDirIterator>
#include <QStyle>

namespace
{
	enum class AssetDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1,
		TYPE,
		IS_MODIFIED,
	};

	constexpr const char* c_project_metadata_folder_name = ".vadon";

	bool is_excluded_asset_directory(const QFileInfo& dir_info)
	{
		QDir directory(dir_info.absoluteFilePath());
		if (directory.dirName() == c_project_metadata_folder_name)
		{
			return true;
		}

		// TODO: check for "ignore" file?
		return false;
	}

	QStandardItem* find_asset_by_id_recursive(const QStandardItem* parent, int id)
	{
		for (int child_index = 0; child_index < parent->rowCount(); ++child_index)
		{
			QStandardItem* current_child = parent->child(child_index);
			if (current_child->data(static_cast<int>(AssetDataRole::ID)).toInt() == id)
			{
				return current_child;
			}

			QStandardItem* found_descendant = find_asset_by_id_recursive(current_child, id);
			if (found_descendant != nullptr)
			{
				return found_descendant;
			}
		}

		return nullptr;
	}

	QStandardItem* create_asset_tree_item(const QString& label)
	{
		QStandardItem* new_item = new QStandardItem(label);
		new_item->setCheckable(false);
		new_item->setEditable(false);
		new_item->setDragEnabled(false);

		return new_item;
	}

	QString get_imported_file_absolute_path(const QFileInfo& resource_file_info)
	{
		// Assume file info is for the .vdimport file
		// The file should be the same path but with the final suffix removed
		return QString("%1/%2").arg(resource_file_info.absolutePath()).arg(resource_file_info.completeBaseName());
	}

	QString get_imported_file_relative_path(const QString& asset_file_path)
	{
		// Assume file info is for the .vdimport file
		// The file should be the same path but with the final suffix removed
		const QFileInfo file_info(asset_file_path);
		return QString("%1/%2").arg(file_info.dir().path()).arg(file_info.completeBaseName());
	}

	QString get_imported_file_resource_path(const QString& imported_file_path)
	{
		// Assume file info is for the .vdimport file
		// The file should be the same path but with the final suffix removed
		return QString("%1.%2").arg(imported_file_path).arg(VadonEditor::Core::AssetInfo::get_file_suffix(VadonEditor::Core::AssetType::IMPORTED_FILE));
	}

	QVariant get_asset_item_data(QStandardItem* asset_item, AssetDataRole role)
	{
		return asset_item->data(static_cast<int>(role));
	}

	void set_asset_item_data(QStandardItem* asset_item, AssetDataRole role, const QVariant& data)
	{
		asset_item->setData(data, static_cast<int>(role));
	}

	QIcon::ThemeIcon get_asset_type_icon(VadonEditor::Core::AssetType type)
	{
		switch (type)
		{
		case VadonEditor::Core::AssetType::RESOURCE:
			return QIcon::ThemeIcon::DocumentNew;
		case VadonEditor::Core::AssetType::SCENE:
			return QIcon::ThemeIcon::CameraVideo;
		case VadonEditor::Core::AssetType::IMPORTED_FILE:
			return QIcon::ThemeIcon::MediaTape;
		case VadonEditor::Core::AssetType::FOLDER:
			return QIcon::ThemeIcon::FolderOpen;
		}

		return QIcon::ThemeIcon::NThemeIcons;
	}

	bool is_imported_file_path_valid(const QString& relative_path)
	{
		if (relative_path.startsWith("..") == true)
		{
			qCritical() << "Cannot import file outside project root!";
			return false;
		}
		else if (relative_path.startsWith(c_project_metadata_folder_name) == true)
		{
			qCritical() << "Cannot import file from project metadata directory!";
			return false;
		}

		const QFileInfo file_info(relative_path);
		for (int asset_type_index = 0; asset_type_index < static_cast<int>(VadonEditor::Core::AssetType::TYPE_COUNT); ++asset_type_index)
		{
			const VadonEditor::Core::AssetType asset_type = static_cast<VadonEditor::Core::AssetType>(asset_type_index);
			if (file_info.suffix() == VadonEditor::Core::AssetInfo::get_file_suffix(asset_type))
			{
				qCritical() << "Imported file cannot be engine resource file!";
				return false;
			}
		}

		if (file_info.fileName() == VadonEditor::Core::ProjectInfo::c_project_file_name)
		{
			qCritical() << "Cannot import the project metadata file!";
			return false;
		}
		
		return true;
	}
}

namespace VadonEditor::Core
{
	QModelIndex AssetManager::create_asset(const AssetInfo& info)
	{
		if (find_asset_by_id(info.id) != nullptr)
		{
			qCritical() << "Asset already exists!";
			return QModelIndex();
		}

		const QFileInfo file_info(get_asset_absolute_file_path(info.path));
		if (file_info.exists() == true)
		{
			qCritical() << "Asset file already exists at" << file_info.absoluteFilePath();
			return QModelIndex();
		}

		InternalAssetInfo internal_asset_info;
		internal_asset_info.type = info.type;

		QStandardItem* new_asset_item = add_asset(info.path, internal_asset_info);
		if (new_asset_item == nullptr)
		{
			qCritical() << "Failed to create asset!";
			return QModelIndex();
		}

		if (info.type == AssetType::FOLDER)
		{
			const QDir directory;
			if (directory.mkpath(file_info.absoluteFilePath()) == false)
			{
				qCritical() << "Failed to create folder at" << file_info.absoluteFilePath() << "!";
				delete new_asset_item;
				return QModelIndex();
			}
		}

		return new_asset_item->index();
	}

	QModelIndex AssetManager::import_asset_file(const QString& file_path)
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();

		const QFileInfo file_info(file_path);
		if (file_info.exists() == false)
		{
			qCritical() << "Imported asset does not exist!";
			return QModelIndex();
		}

		if (file_info.isDir() == true)
		{
			qCritical() << "Path to import is a folder!";
			return QModelIndex();
		}

		QDir project_root_dir(project_info.root_path);
		const QString relative_path = project_root_dir.relativeFilePath(file_path);

		if (relative_path.startsWith(".."))
		{
			qCritical() << "Cannot import file outside project root!";
			return QModelIndex();
		}

		if (is_imported_file_path_valid(relative_path) == false)
		{
			return QModelIndex();
		}

		// Check whether it's an existing asset
		{
			const QModelIndex existing_index = find_asset_index_by_path(relative_path);
			if (existing_index.isValid() == true)
			{
				qWarning() << "Asset already registered in the project!";
				return existing_index;
			}
		}

		const QString import_resource_file_path = file_path + "." + AssetInfo::get_file_suffix(AssetType::IMPORTED_FILE);
		const QFileInfo import_resource_info(import_resource_file_path);
		if (import_resource_info.exists() == true)
		{
			qCritical() << "Import resource file already exists!";
			return QModelIndex();
		}

		QFile import_resource_file(import_resource_info.absoluteFilePath());
		if (import_resource_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to create import file" << file_info.absoluteFilePath();
			return QModelIndex();
		}

		InternalAssetInfo imported_asset_info;
		imported_asset_info.type = AssetType::IMPORTED_FILE;

		QStandardItem* new_asset_item = add_asset(relative_path, imported_asset_info);
		if (new_asset_item == nullptr)
		{
			qCritical() << "Failed to create asset!";
			return QModelIndex();
		}

		return new_asset_item->index();
	}

	AssetInfo AssetManager::get_asset_info(const QModelIndex& index) const
	{
		AssetInfo asset_info;

		if (index.isValid() == false)
		{
			return asset_info;
		}

		QStandardItem* asset_item = m_asset_model.itemFromIndex(index);
		if ((asset_item == nullptr) || (asset_item == m_asset_model.invisibleRootItem()))
		{
			return asset_info;
		}

		asset_info.path = get_asset_path(asset_item);
		asset_info.id = get_asset_item_data(asset_item, AssetDataRole::ID).toInt();
		asset_info.type = static_cast<AssetType>(get_asset_item_data(asset_item, AssetDataRole::TYPE).toInt());

		return asset_info;
	}

	void AssetManager::set_asset_modified(const QModelIndex& index, bool modified)
	{
		QStandardItem* asset_item = m_asset_model.itemFromIndex(index);
		Q_ASSERT_X((asset_item != nullptr) && (asset_item != m_asset_model.invisibleRootItem()), "VadonEditor::Core::AssetManager::set_asset_modified", "Invalid item!");

		QBrush text_brush = QApplication::style()->standardPalette().brush(QPalette::ColorRole::Text);
		if (modified)
		{
			text_brush.setColor(Qt::GlobalColor::yellow);
		}

		asset_item->setForeground(text_brush);
	}

	bool AssetManager::save_asset_data(int asset_id, QByteArrayView data)
	{
		const QModelIndex asset_index = find_asset_index(asset_id);
		if (asset_index.isValid() == false)
		{
			qCritical() << "Asset not registered!";
			return false;
		}

		const QStandardItem* asset_item = m_asset_model.itemFromIndex(asset_index);
		const QString asset_path = get_asset_path(asset_item);

		QString asset_absolute_path = get_asset_absolute_file_path(asset_path);

		const AssetInfo asset_info = get_asset_info(asset_index);
		if (asset_info.type == AssetType::IMPORTED_FILE)
		{
			// Redirect to the import resource
			asset_absolute_path = get_imported_file_resource_path(asset_absolute_path);
		}

		const QFileInfo file_info(asset_absolute_path);
		if (file_info.exists() == false)
		{
			// File doesn't exist yet, attempt to create the path to it
			const QDir asset_dir;
			if (asset_dir.mkpath(file_info.absolutePath()) == false)
			{
				qCritical() << "Failed to create path" << file_info.absolutePath();
				return false;
			}
		}

		QFile asset_file(file_info.absoluteFilePath());
		if (asset_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to open asset file" << file_info.absoluteFilePath();
			return false;
		}

		if (asset_file.write(data.data(), data.length()) != data.length())
		{
			qCritical() << "Failed to write all data to file" << file_info.absoluteFilePath();
			return false;
		}
		return true;
	}

	bool AssetManager::load_asset_data(int asset_id, QByteArray& data) const
	{
		const QModelIndex asset_index = find_asset_index(asset_id);
		const QStandardItem* asset_item = m_asset_model.itemFromIndex(asset_index);

		const QString asset_path = get_asset_path(asset_item);
		QString asset_absolute_path = get_asset_absolute_file_path(asset_path);

		const AssetInfo asset_info = get_asset_info(asset_index);
		if (asset_info.type == AssetType::IMPORTED_FILE)
		{
			// Redirect to the import resource
			asset_absolute_path = get_imported_file_resource_path(asset_absolute_path);
		}

		const QFileInfo file_info(asset_absolute_path);
		if (file_info.exists() == false)
		{
			qCritical() << "Cannot find asset file" << file_info.absoluteFilePath();
			return false;
		}

		QFile asset_file(file_info.absoluteFilePath());
		if (asset_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open asset file" << file_info.absoluteFilePath();
			return false;
		}

		data = asset_file.readAll();
		return true;
	}

	bool AssetManager::load_imported_file_data(int asset_id, QByteArray& data) const
	{
		// TODO: load the actual file data instead of the import resource
		Q_UNUSED(asset_id);
		Q_UNUSED(data);
		return false;
	}

	QModelIndex AssetManager::find_asset_index(int asset_id) const
	{
		QStandardItem* asset_item = find_asset_by_id(asset_id);
		if ((asset_item != nullptr) && (asset_item != m_asset_model.invisibleRootItem()))
		{
			return asset_item->index();
		}

		return QModelIndex();
	}

	QModelIndex AssetManager::find_asset_index_by_path(const QString& path) const
	{
		QStandardItem* asset_item = find_asset_by_path(path);
		if ((asset_item != nullptr) && (asset_item != m_asset_model.invisibleRootItem()))
		{
			return asset_item->index();
		}

		return QModelIndex();
	}

	void AssetManager::open_asset(const QModelIndex& index)
	{
		if (index.isValid() == false)
		{
			return;
		}

		emit asset_opened(index);
	}

	AssetManager::AssetManager(Application& application)
		: m_application(application)
		, m_id_counter(0)
	{
	}

	bool AssetManager::initialize()
	{
		// TODO: use a QFileSystemModel to watch for file changes?
		return true;
	}

	void AssetManager::project_loaded()
	{
		if (m_application.get_configuration().mode != ApplicationMode::EDITOR)
		{
			// We only manage assets in the editor
			return;
		}

		const Core::ProjectManager& project_manager = m_application.get_project_manager();
		if (project_manager.get_project_data_schema().is_valid() == false)
		{
			// FIXME: some way to load assets even without a data schema?
			// This may need to be a bare minimum requirement, they don't need to run the
			// engine plugin to work on assets, but they do need to at least generate the schema
			qWarning() << "Asset Manager cannot load assets without valid data schema!";
			return;
		}

		const ProjectInfo& project_info = project_manager.get_project_info();

		QHash<QString, AssetType> asset_type_lookup;
		for (int type_index = 0; type_index < static_cast<int>(AssetType::TYPE_COUNT); ++type_index)
		{
			const AssetType current_type = static_cast<AssetType>(type_index);
			QString suffix = AssetInfo::get_file_suffix(current_type);
			if (suffix.isEmpty() == false)
			{
				asset_type_lookup.insert(suffix, current_type);
			}
		}

		// NOTE: root folder needs to be a special case that duplicates the logic,
		// because we don't want to accidentally add the root folder itself
		QDirIterator dir_iterator(project_info.root_path, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
		while (dir_iterator.hasNext())
		{
			dir_iterator.next();
			const QFileInfo child_info = dir_iterator.fileInfo();
			scan_assets_recursive(child_info, asset_type_lookup);
		}
	}

	void AssetManager::scan_assets_recursive(const QFileInfo& file_info, const QHash<QString, AssetType>& asset_type_lookup)
	{
		if (file_info.isDir() == true)
		{
			if (is_excluded_asset_directory(file_info))
			{
				return;
			}

			// Add the folder as an asset
			InternalAssetInfo folder_info;
			folder_info.type = AssetType::FOLDER;
			add_asset(get_asset_relative_path(file_info.absoluteFilePath()), folder_info);

			QDirIterator dir_iterator(file_info.absoluteFilePath(), QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
			while (dir_iterator.hasNext())
			{
				dir_iterator.next();
				const QFileInfo child_info = dir_iterator.fileInfo();
				scan_assets_recursive(child_info, asset_type_lookup);
			}
		}
		else
		{
			auto asset_type_it = asset_type_lookup.find(file_info.suffix());
			if (asset_type_it != asset_type_lookup.end())
			{
				const AssetType asset_type = asset_type_it.value();

				InternalAssetInfo asset_info;
				asset_info.type = asset_type;

				switch (asset_type)
				{
				case AssetType::RESOURCE:
				case AssetType::SCENE:
				case AssetType::IMPORTED_FILE:
				{
					QString asset_path = get_asset_relative_path(file_info.absoluteFilePath());
					if (asset_type == AssetType::IMPORTED_FILE)
					{
						// Make sure the imported file also exists
						// Get path to the imported file by removing the import suffix
						const QString imported_file_path = get_imported_file_absolute_path(file_info);
						if (QFileInfo(imported_file_path).exists() == false)
						{
							qWarning() << "Found import file at" << file_info.absoluteFilePath() << "but no corresponding asset file!";
							return;
						}
						asset_path = get_imported_file_relative_path(asset_path);
					}

					// Add asset, delegate validation to other systems
					asset_info.id = generate_new_asset_id();
					add_asset(asset_path, asset_info);
				}
				break;
				// TODO: other asset types?
				}
			}
		}
	}

	QStandardItem* AssetManager::add_asset(const QString& path, const InternalAssetInfo& info)
	{
		Q_ASSERT_X(path.isEmpty() == false, "VadonEditor::Core::AssetManager::add_asset", "Asset must have valid path");
		if (find_asset_by_path(path) != nullptr)
		{
			qCritical() << "Asset already added at path" << path;
			return nullptr;
		}

		QFileInfo file_info(path);
		QStandardItem* parent_item = find_asset_by_path(file_info.dir().path());
		Q_ASSERT_X(parent_item != nullptr, "VadonEditor::Core::AssetManager::add_asset", "Parent not added");

		QStandardItem* new_asset_item = create_asset_tree_item(file_info.fileName());
		set_asset_item_data(new_asset_item, AssetDataRole::ID, generate_new_asset_id());
		set_asset_item_data(new_asset_item, AssetDataRole::TYPE, static_cast<int>(info.type));
		new_asset_item->setIcon(QIcon::fromTheme(get_asset_type_icon(info.type)));

		parent_item->appendRow(new_asset_item);

		return new_asset_item;
	}

	QStandardItem* AssetManager::find_asset_by_path(const QString& path) const
	{
		if (path == '.')
		{
			// Relative path that is "dot" is the root item
			return m_asset_model.invisibleRootItem();
		}

		QStringList split_path = QDir::cleanPath(path).split(AssetInfo::c_dir_separator);

		QStandardItem* current_parent = m_asset_model.invisibleRootItem();
		for (const QString& path_part : split_path)
		{
			QStandardItem* found_child = nullptr;
			for (int child_index = 0; (child_index < current_parent->rowCount()) && (found_child == nullptr); ++child_index)
			{
				QStandardItem* current_child = current_parent->child(child_index);
				if (current_child->text() == path_part)
				{
					found_child = current_child;
				}
			}
			if (found_child == nullptr)
			{
				return nullptr;
			}
			else
			{
				current_parent = found_child;
			}
		}
		return current_parent;
	}

	QStandardItem* AssetManager::find_asset_by_id(int id) const
	{
		if (id == AssetInfo::c_invalid_file_id)
		{
			return nullptr;
		}

		return find_asset_by_id_recursive(m_asset_model.invisibleRootItem(), id);
	}

	QString AssetManager::get_asset_path(const QStandardItem* asset_item) const
	{
		QString path = asset_item->text();
		QStandardItem* parent = asset_item->parent();
		while ((parent != nullptr) && (parent != m_asset_model.invisibleRootItem()))
		{
			path = parent->text() + AssetInfo::c_dir_separator + path;
			parent = parent->parent();
		}

		return path;
	}

	QString AssetManager::get_asset_absolute_file_path(const QString& asset_path) const
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();
		return QDir::cleanPath(project_info.root_path + AssetInfo::c_dir_separator + asset_path);
	}

	QString AssetManager::get_asset_relative_path(const QString& asset_path) const
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();
		QDir project_root_dir(project_info.root_path);
		return QDir::cleanPath(project_root_dir.relativeFilePath(asset_path));
	}

	int AssetManager::generate_new_asset_id()
	{
		const int new_asset_id = (m_id_counter + 1);
		++m_id_counter;

		return new_asset_id;
	}
}