#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/Asset.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Resource/File.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>

#include <QDirIterator>

namespace
{
	constexpr char c_vadon_dir_separator = '/';

	enum class AssetDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1, 
		TYPE,
		
	};

	bool is_excluded_asset_directory(const QFileInfo& dir_info)
	{
		QDir directory(dir_info.absoluteFilePath());
		if (directory.dirName() == ".vadon")
		{
			return true;
		}

		// TODO: check for "ignore" file?
		return false;
	}

	QStandardItem* find_asset_by_id_recursive(const QStandardItem* parent, const QUuid& id)
	{
		for (int child_index = 0; child_index < parent->rowCount(); ++child_index)
		{
			QStandardItem* current_child = parent->child(child_index);
			if (current_child->data(static_cast<int>(AssetDataRole::ID)).toUuid() == id)
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
}

namespace VadonEditor::Core
{
	QModelIndex AssetManager::create_asset(const AssetInfo& info)
	{
		if (find_asset_by_id(info.file_id) != nullptr)
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
		internal_asset_info.file_id = info.file_id;
		internal_asset_info.type = info.type;

		QStandardItem* new_asset_item = add_asset(info.path, internal_asset_info);
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
		asset_info.file_id = asset_item->data(static_cast<int>(AssetDataRole::ID)).toUuid();
		asset_info.type = static_cast<AssetType>(asset_item->data(static_cast<int>(AssetDataRole::TYPE)).toInt());

		return asset_info;
	}

	bool AssetManager::save_asset_data(const QUuid& asset_id, QByteArrayView data)
	{
		QStandardItem* asset_item = find_asset_by_id(asset_id);
		if (asset_item == nullptr)
		{
			qCritical() << "Asset not registered!";
			return false;
		}

		const QString asset_path = get_asset_path(asset_item);

		const QFileInfo file_info(get_asset_absolute_file_path(asset_path));
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

	bool AssetManager::load_asset_data(const QUuid& asset_id, QByteArray& data) const
	{
		QStandardItem* asset_item = find_asset_by_id(asset_id);

		const QString asset_path = get_asset_path(asset_item);
		const QFileInfo file_info(get_asset_absolute_file_path(asset_path));
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
	{
	}

	bool AssetManager::initialize()
	{
		// TODO: use a QFileSystemModel to watch for file changes?
		return true;
	}

	void AssetManager::project_loaded()
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();

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
					if (asset_type == AssetType::IMPORTED_FILE)
					{
						// Make sure the imported file also exists
						// Get path to the imported file by removing the import suffix
						const QString imported_file_path = Model::ResourceSystem::get_imported_file_path(file_info);
						if (QFileInfo(imported_file_path).exists() == false)
						{
							qWarning() << "Found import file at" << file_info.absoluteFilePath() << "but no corresponding asset file!";
							return;
						}
					}

					// FIXME: use a cache so we don't need to load the files every time
					QFile resource_file(file_info.absoluteFilePath());
					if (resource_file.open(QIODevice::ReadOnly) == false)
					{
						qCritical() << "Failed to open resource file!";
						return;
					}

					QByteArray resource_file_data = resource_file.readAll();

					const Model::ResourceInfo resource_info = m_application.get_model_system().get_resource_system().parse_resource_info(resource_file_data);
					asset_info.file_id = resource_info.id;

					if (asset_info.file_id.isNull() == false)
					{
						switch (asset_type)
						{
						case AssetType::SCENE:
						{
							if (resource_info.type != Utilities::base64_string_to_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid.string))
							{
								qCritical() << "Scene file" << file_info.absoluteFilePath() << "does not contain scene type!";
								return;
							}
						}
						break;
						case AssetType::IMPORTED_FILE:
						{
							if (resource_info.type != Utilities::base64_string_to_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid.string))
							{
								qCritical() << "Imported file" << file_info.absoluteFilePath() << "does not contain imported file resource type!";
								return;
							}
						}
						break;
						}

						add_asset(get_asset_relative_path(file_info.absoluteFilePath()), asset_info);
					}
					else
					{
						qCritical() << "Invalid asset file data in" << file_info.absoluteFilePath();
					}
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
		new_asset_item->setData(info.file_id, static_cast<int>(AssetDataRole::ID));
		new_asset_item->setData(static_cast<int>(info.type), static_cast<int>(AssetDataRole::TYPE));

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

		QStringList split_path = QDir::cleanPath(path).split(c_vadon_dir_separator);

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

	QStandardItem* AssetManager::find_asset_by_id(const QUuid& id) const
	{
		if (id.isNull() == true)
		{
			return nullptr;
		}

		return find_asset_by_id_recursive(m_asset_model.invisibleRootItem(), id);
	}

	QString AssetManager::get_asset_path(QStandardItem* asset_item) const
	{
		QString path = asset_item->text();
		QStandardItem* parent = asset_item->parent();
		while ((parent != nullptr) && (parent != m_asset_model.invisibleRootItem()))
		{
			path = parent->text() + c_vadon_dir_separator + path;
			parent = parent->parent();
		}

		return path;
	}

	QString AssetManager::get_asset_absolute_file_path(const QString& asset_path) const
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();
		return QDir::cleanPath(project_info.root_path + c_vadon_dir_separator + asset_path);
	}

	QString AssetManager::get_asset_relative_path(const QString& asset_path) const
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();
		QDir project_root_dir(project_info.root_path);
		return QDir::cleanPath(project_root_dir.relativeFilePath(asset_path));
	}
}