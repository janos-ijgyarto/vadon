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

namespace
{
	enum class AssetDataRole
	{
		ID = Qt::ItemDataRole::UserRole + 1, 
		TYPE
	};

	QString get_asset_full_path(const VadonEditor::Core::AssetInfo& asset_info)
	{
		// TODO: combine the parts into a full path
		return QString();
	}
}

namespace VadonEditor::Core
{
	void AssetFilterModel::add_asset(const QString& path, const InternalAssetInfo& info)
	{
		// TODO: check whether asset was already added!
		m_asset_lookup.insert(path, info);
		m_file_lookup.insert(info.file_id, path);
		invalidate();
	}

	void AssetFilterModel::remove_asset(const QString& path)
	{
		// TODO: check whether asset was present!
		m_asset_lookup.remove(path);
		invalidate();
	}

	AssetFilterModel::InternalAssetInfo AssetFilterModel::get_asset_info(const QModelIndex& index) const
	{
		const QString file_path = sourceModel()->data(index, QFileSystemModel::Roles::FilePathRole).toString();
		
		auto asset_it = m_asset_lookup.find(file_path);		
		if (asset_it == m_asset_lookup.end())
		{
			return AssetFilterModel::InternalAssetInfo{};
		}

		return asset_it.value();
	}

	QString AssetFilterModel::find_asset_file(const QUuid& file_id) const
	{
		auto path_it = m_file_lookup.find(file_id);
		if (path_it == m_file_lookup.end())
		{
			return QString();
		}

		return path_it.value();
	}

	bool AssetFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
	{
		QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);

		const QString file_path = sourceModel()->data(source_index, QFileSystemModel::Roles::FilePathRole).toString();
		
		return m_asset_lookup.contains(file_path);
	}

	AssetFilterModel::AssetFilterModel()
	{

	}

	bool AssetManager::create_asset(const AssetInfo& info)
	{
		const QString existing_file = m_asset_filter_model.find_asset_file(info.file_id);
		if (existing_file.isEmpty() == false)
		{
			qCritical() << "Asset already registered with this UUID!";
			return false;
		}

		const QFileInfo file_info(get_asset_full_path(info));
		if (file_info.exists() == true)
		{
			qCritical() << "Asset file already exists at \"" << file_info.absoluteFilePath() << "\"!";
			return false;
		}

		// TODO: if checks pass, add to lookup
		m_asset_filter_model;
		
		return true;
	}

	AssetInfo AssetManager::get_asset_info(const QModelIndex& index) const
	{
		AssetInfo asset_info;

		if (index.isValid() == false)
		{
			return asset_info;
		}

		const AssetFilterModel::InternalAssetInfo internal_info = m_asset_filter_model.get_asset_info(index);

		// TODO: convert to public asset info object
		asset_info.file_id = internal_info.file_id;
		asset_info.name;

		return asset_info;
	}

	bool AssetManager::save_asset_data(const QUuid& asset_id, QByteArrayView data)
	{
		const QString asset_file_path = m_asset_filter_model.find_asset_file(asset_id);
		if (asset_file_path.isEmpty() == true)
		{
			qCritical() << "Asset not registered!";
			return false;
		}

		const QFileInfo file_info(asset_file_path);
		if (file_info.exists() == false)
		{
			// File doesn't exist yet, attempt to create the path to it
			const QDir asset_dir;
			if (asset_dir.mkpath(file_info.absolutePath()) == false)
			{
				qCritical() << "Failed to create path \"" << file_info.absolutePath() << "\"!";
				return false;
			}
		}

		QFile asset_file(file_info.absoluteFilePath());
		if (asset_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to open asset file \"" << file_info.absoluteFilePath() << "\"!";
			return false;
		}

		if (asset_file.write(data.data(), data.length()) != data.length())
		{
			qCritical() << "Failed to write all data to file \"" << file_info.absoluteFilePath() << "\"!";
			return false;
		}
		return true;
	}

	bool AssetManager::load_asset_data(const QUuid& asset_id, QByteArray& data) const
	{
		const QFileInfo file_info(m_asset_filter_model.find_asset_file(asset_id));
		if (file_info.exists() == false)
		{
			qCritical() << "Cannot find asset file \"" << file_info.absoluteFilePath() << "\"!";
			return false;
		}

		QFile asset_file(file_info.absoluteFilePath());
		if (asset_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open asset file \"" << file_info.absoluteFilePath() << "\"!";
			return false;
		}

		data = asset_file.readAll();
		return true;
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
		// TODO: set up model to also watch for changes!
		QObject::connect(&m_file_model, &QFileSystemModel::directoryLoaded,
			[this](const QString& directory)
			{
				directory_loaded(directory);
			}
		);

		return true;
	}

	bool AssetManager::project_loaded()
	{
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();
		m_file_model.setRootPath(project_info.root_path);

		return true;
	}

	void AssetManager::directory_loaded(const QString& path)
	{
		// TODO: implement cache so we don't have to keep loading all the files
		const ProjectInfo& project_info = m_application.get_project_manager().get_project_info();

		if (path != project_info.root_path)
		{
			return;
		}

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

		auto enumerate_files = [this, &asset_type_lookup](const QString& path) -> void {
			auto do_enumerate_files = [this, &asset_type_lookup](const auto& self_func, const QString& path) -> bool {
				const QModelIndex root_index = m_file_model.index(path);

				const QFileInfo file_info = m_file_model.fileInfo(root_index);
				if (file_info.isDir() == true)
				{
					const int row_count = m_file_model.rowCount(root_index);
					bool has_accepted_file = false;
					for (int row_index = 0; row_index < row_count; ++row_index)
					{
						const QModelIndex child_index = m_file_model.index(row_index, 0, root_index);
						const QString child_path = m_file_model.filePath(child_index);
						has_accepted_file |= self_func(self_func, child_path);
					}
					if (has_accepted_file)
					{
						AssetFilterModel::InternalAssetInfo asset_info;
						asset_info.type = AssetType::FOLDER;
						m_asset_filter_model.add_asset(path, asset_info);
					}
					return has_accepted_file;
				}
				else
				{
					auto asset_type_it = asset_type_lookup.find(file_info.suffix());
					if (asset_type_it != asset_type_lookup.end())
					{
						const AssetType asset_type = asset_type_it.value();

						AssetFilterModel::InternalAssetInfo asset_info;
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
								const QModelIndex imported_file_index = m_file_model.index(imported_file_path);
								if (imported_file_index.isValid() == false)
								{
									qWarning() << "Found import file at \"" << file_info.absoluteFilePath() << "\" but no corresponding asset file!";
									return false;
								}
							}

							// FIXME: use a cache so we don't need to load the files every time
							QFile resource_file(file_info.absoluteFilePath());
							if (resource_file.open(QIODevice::ReadOnly) == false)
							{
								qCritical() << "Failed to open resource file!";
								return false;
							}

							QByteArray resource_file_data =	resource_file.readAll();

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
										qCritical() << "Scene file \"" << file_info.absoluteFilePath() << "\" does not contain scene type!";
										return false;
									}
								}
								break;
								case AssetType::IMPORTED_FILE:
								{
									if (resource_info.type != Utilities::base64_string_to_uuid(::Vadon::Foundation::FileResourceSchema::c_type_uuid.string))
									{
										qCritical() << "Scene file \"" << file_info.absoluteFilePath() << "\" does not contain scene type!";
										return false;
									}
								}
								break;
								}

								m_asset_filter_model.add_asset(file_info.absoluteFilePath(), asset_info);
								return true;
							}
							else
							{
								qCritical() << "Invalid asset file data in \"" << file_info.absoluteFilePath() << "\"";
								return false;
							}
						}
							break;
							// TODO: other asset types?
						}
					}

					return false;
				}
			};
			do_enumerate_files(do_enumerate_files, path);
		};

		enumerate_files(project_info.root_path);

		m_asset_filter_model.setSourceModel(&m_file_model);
	}
}