#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>
#include <VadonEditor/Core/Configuration.hpp>

#include <VadonEditor/Core/Plugin/Plugin.hpp>
#include <VadonEditor/Core/Plugin/PluginManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Editor/Simulator/LibraryInterface.hpp>
#include <Vadon/Foundation/Project/Project.hpp>

#include <QCoreApplication>

#include <QDir>
#include <QDirIterator>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QProcess>

#include <QSettings>

namespace
{
	constexpr const char* c_project_cache_prefix = "ProjectManager/project_cache/projects";

	constexpr const char* c_editor_plugin_suffix = "vdeplugin";
	constexpr const char* c_game_executable_suffix = "vdgexe";

	enum class ApplicationSetting
	{
		NAME,
		PATH,
		SETTINGS_COUNT
	};

	constexpr const char* c_app_settings_keys[static_cast<size_t>(ApplicationSetting::SETTINGS_COUNT)] = {
		"name",
		"path"
	};

	constexpr const char* get_app_settings_key(ApplicationSetting setting) { return c_app_settings_keys[static_cast<size_t>(setting)]; }

	enum class ProjectEditorPluginSetting
	{
		CUSTOM_SEARCH_PATH,
		SELECTED_CONFIGURATION,
		SETTINGS_COUNT
	};

	constexpr const char* c_project_editor_plugin_settings_keys[static_cast<size_t>(ProjectEditorPluginSetting::SETTINGS_COUNT)] = {
		"editor_plugin_custom_search_path",
		"editor_plugin_selected_configuration"
	};

	constexpr const char* get_editor_plugin_settings_key(ProjectEditorPluginSetting setting) { return c_project_editor_plugin_settings_keys[static_cast<size_t>(setting)]; }

	enum class ProjectGameExecutableSetting
	{
		CUSTOM_SEARCH_PATH,
		SELECTED_CONFIGURATION,
		SETTINGS_COUNT
	};

	constexpr const char* c_project_game_executable_settings_keys[static_cast<size_t>(ProjectGameExecutableSetting::SETTINGS_COUNT)] = {
		"game_executable_custom_search_path",
		"game_executable_selected_configuration"
	};

	constexpr const char* get_game_executable_settings_key(ProjectGameExecutableSetting setting) { return c_project_game_executable_settings_keys[static_cast<size_t>(setting)]; }

	bool validate_project_name(const QString& name)
	{
		if (name.isEmpty() == true)
		{
			return false;
		}

		// TODO: illegal characters, etc?
		return true;
	}

	bool validate_project_file(const QFileInfo& project_file)
	{
		if (project_file.exists() != true)
		{
			return false;
		}

		if (project_file.isFile() != true)
		{
			return false;
		}

		if (project_file.suffix() != "vdpr")
		{
			return false;
		}

		return true;
	}

	bool validate_project_plugin_path(const QString& plugin_path)
	{
		if (plugin_path.isEmpty() == true)
		{
			// No plugin path is valid!
			return true;
		}

		const QFileInfo plugin_info(plugin_path);
		if ((plugin_info.exists() == false) || (plugin_info.isFile() == false))
		{
			return false;
		}

		// FIXME: allow other platforms to use a different suffix!
		if (plugin_info.completeSuffix() != "dll")
		{
			return false;
		}

		return true;
	}

	bool validate_project_info(const VadonEditor::Core::ProjectInfo& project_info)
	{
		// TODO: anything else to validate?
		return validate_project_name(project_info.name);
	}

	bool load_project_info(const QJsonDocument& json_doc, VadonEditor::Core::ProjectInfo& project_info)
	{
		if (json_doc.isNull() == true)
		{
			// TODO: more detailed error!
			qCritical() << "Project file contains invalid data!";
			return false;
		}

		const QUuid project_name_property_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);
		const QUuid custom_data_resource_property_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_resource_property.id);

		// TODO: validate JSON data!
		const QJsonObject project_info_root = json_doc.object();

		for (auto property_it = project_info_root.begin(); property_it != project_info_root.end(); ++property_it)
		{
			const QUuid property_uuid = VadonEditor::Utilities::parse_labeled_uuid(property_it.key());
			if (property_uuid == project_name_property_id)
			{
				project_info.name = property_it.value().toString();
			}
			else if (property_uuid == custom_data_resource_property_id)
			{
				project_info.custom_data_resource_id = VadonEditor::Utilities::base64_string_to_uuid(property_it.value().toString());
			}
		}
		
		return true;
	}

	bool load_editor_plugin_info(const QJsonDocument& plugin_json, VadonEditor::Core::EditorPluginInfo& plugin_info)
	{
		if (plugin_json.isNull() == true)
		{
			// TODO: more detailed error!
			qCritical() << "Editor plugin file contains invalid data!";
			return false;
		}

		const QJsonObject plugin_info_root = plugin_json.object();

		if (auto config_it = plugin_info_root.constFind("configuration"); config_it != plugin_info_root.end())
		{
			if (config_it->isString() == false)
			{
				return false;
			}

			plugin_info.configuration_name = config_it->toString();
		}

		// TODO: any other data?
		return true;
	}

	bool load_game_executable_info(const QJsonDocument& executable_json, VadonEditor::Core::GameExecutableInfo& executable_info)
	{
		if (executable_json.isNull() == true)
		{
			// TODO: more detailed error!
			qCritical() << "Game executable file contains invalid data!";
			return false;
		}

		const QJsonObject executable_info_root = executable_json.object();

		if (auto config_it = executable_info_root.constFind("configuration"); config_it != executable_info_root.end())
		{
			if (config_it->isString() == false)
			{
				return false;
			}

			executable_info.configuration_name = config_it->toString();
		}

		// TODO: any other data?
		return true;
	}

	QString get_project_data_schema_path(const VadonEditor::Core::ProjectInfo& project_info)
	{
		return QString("%1/.vadon/data_schema.json").arg(project_info.root_path);
	}
}

namespace VadonEditor::Core
{
	void ProjectManager::set_project_info(const ProjectInfo& project_info)
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::set_project_info", "Project not loaded");
		Q_ASSERT_X(validate_project_name(project_info.name), "ProjectManager::set_project_info", "Invalid name");

		m_loaded_project_info = project_info;

		save_current_project_data();

		// Update cache entry
		{
			auto cache_it = m_project_cache.find(project_info.root_path);
			if (cache_it == m_project_cache.end())
			{
				qCritical() << "Project not in cache!";
				return;
			}

			// TODO: also save selected config!
			CachedProjectInfo& cached_info = cache_it.value();
			cached_info.plugin_settings.custom_search_path = project_info.plugin_settings.custom_search_path;
			cached_info.plugin_settings.selected_config = project_info.plugin_settings.selected_configuration;

			cached_info.game_settings.custom_search_path = project_info.game_settings.custom_search_path;
			cached_info.game_settings.selected_config = project_info.game_settings.selected_configuration;

			save_project_cache();
		}
	}

	bool ProjectManager::generate_project_data_schema(const QString& plugin_config)
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::generate_project_data_schema", "Project not loaded");
		const Core::ProjectInfo& project_info = get_project_info();

		const EditorPluginInfo* editor_plugin_info = project_info.find_plugin_entry(plugin_config);
		if (editor_plugin_info == nullptr)
		{
			qCritical() << "Invalid setting for project editor plugin!";
			return false;
		}

		switch (m_application.get_configuration().mode)
		{
		case Core::ApplicationMode::EDITOR:
		{
			// Start process to export the schema
			QProcess exporter_process;

			QString program_path = QCoreApplication::applicationFilePath();
			exporter_process.setProgram(program_path);

			QStringList arguments{ QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::IS_SCHEMA_EXPORTER)) };
			arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::STARTUP_PROJECT_PATH)));
			arguments.push_back(project_info.get_project_file_path());

			arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::PLUGIN_CONFIG_NAME)));
			arguments.push_back(plugin_config);

			arguments.push_back(QString("--%1").arg(Core::CommandLineState::get_parameter_key(Core::CommandLineParameter::DEBUG_BREAK_ON_INIT)));

			exporter_process.setArguments(arguments);

			QObject::connect(&exporter_process, &QProcess::readyReadStandardOutput,
				[&]()
				{
					qInfo() << "SCHEMA EXPORTER: " << qPrintable(exporter_process.readAllStandardOutput().trimmed());
				}
			);
			QObject::connect(&exporter_process, &QProcess::readyReadStandardError,
				[&]()
				{
					qWarning() << "SCHEMA EXPORTER: " << qPrintable(exporter_process.readAllStandardError().trimmed());
				}
			);

			// Start process
			exporter_process.start(QIODevice::ReadOnly);

			// Wait for process to finish
			if (exporter_process.waitForFinished() == false)
			{
				qCritical() << "Error shutting down simulator!";
			}

			if (exporter_process.exitStatus() == QProcess::ExitStatus::NormalExit)
			{
				qDebug() << "Schema exporter process exited with " << exporter_process.exitCode();
				if (exporter_process.exitCode() != 0)
				{
					qCritical() << "Data schema export failed (return code: " << exporter_process.exitCode() << ")";
					return false;
				}
			}
			else
			{
				qCritical() << "Schema exporter process crashed!";
				return false;
			}
		}
		break;
		case Core::ApplicationMode::SCHEMA_EXPORTER:
		{
			VadonEditor::Core::PluginManager& plugin_manager = m_application.get_plugin_manager();

			Core::PluginInfo plugin_info;
			plugin_info.path = editor_plugin_info->path;

			Core::PluginHandle plugin_handle = plugin_manager.load_plugin(plugin_info);
			if (plugin_handle == Core::PluginManager::c_invalid_plugin_handle)
			{
				qCritical() << "Failed to load plugin to export data schema!";
				return false;
			}

			VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginExportDataSchema) export_data_schema_ptr = reinterpret_cast<VADONEDITOR_API_FUNCTION_POINTER(VadonEditorPluginExportDataSchema)>(plugin_manager.get_plugin_function(plugin_handle, VADONEDITOR_API_FUNCTION_NAME(VadonEditorPluginExportDataSchema)));
			if (export_data_schema_ptr == nullptr)
			{
				qCritical() << "Failed to get export schema function address!";
				plugin_manager.unload_plugin(plugin_handle);
				return false;
			}

			// Plugin is loaded, pass in the schema to gather all the types
			export_data_schema_ptr(&m_loaded_project_schema.get_registry());

			// Data is exported, we can unload the plugin
			plugin_manager.unload_plugin(plugin_handle);

			if (m_loaded_project_schema.save_schema(get_project_data_schema_path(project_info)) == false)
			{
				qCritical() << "Failed to save data schema!";
				return false;
			}
		}
		break;
		}

		return true;
	}

	bool ProjectManager::load_project_data_schema()
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::load_project_data_schema", "Project not loaded");
		const Core::ProjectInfo& project_info = get_project_info();

		// NOTE: load into temporary object, only replace the one in system if the load was successful
		if (m_loaded_project_schema.load_schema(get_project_data_schema_path(project_info)) == false)
		{
			qCritical() << "Failed to load data schema!";
			return false;
		}

		return true;
	}

	const QList<ProjectManager::CachedProjectInfo> ProjectManager::get_cached_project_list() const
	{
		QList<ProjectManager::CachedProjectInfo> project_list;
		for (auto cache_it = m_project_cache.begin(); cache_it != m_project_cache.end(); ++cache_it)
		{
			project_list.append(cache_it.value());
		}

		return project_list;
	}

	bool ProjectManager::create_project(const ProjectInfo& project_info)
	{
		if (validate_project_info(project_info) == false)
		{
			qCritical() << "Invalid project info!";
			return false;
		}

		const QFileInfo path_info(project_info.root_path);

		if (path_info.isDir() == false)
		{
			qCritical() << "Invalid root path!";
			return false;
		}

		const QFileInfo project_file_info(path_info.absoluteFilePath() + "/project.vdpr");

		const QString project_file_path = project_file_info.absoluteFilePath();
		for (const CachedProjectInfo& cached_project : m_project_cache)
		{
			if (cached_project.path == project_file_path)
			{
				qCritical() << "Project already in cache!";
				return false;
			}
		}

		if ((project_file_info.exists() == true) && (project_file_info.isFile() == true))
		{
			// Project already exists!
			qCritical() << "Project already exists at destination!";
			return false;
		}

		// Create file by saving project info
		if (internal_save_project(project_file_info, project_info) == false)
		{
			return false;
		}

		// Add to project cache
		CachedProjectInfo cached_info;
		cached_info.name = project_info.name;
		cached_info.path = project_file_path;

		if(add_project_to_cache(project_file_info.absolutePath(), cached_info) == false)
		{
			// TODO: report error?
		}

		return true;
	}

	bool ProjectManager::import_project(const QString& project_path)
	{
		const QFileInfo path_info(project_path);

		if (validate_project_file(path_info) == false)
		{
			qCritical() << "Invalid project path!";
			return false;
		}

		ProjectInfo imported_project_info;
		imported_project_info.root_path = path_info.absolutePath();

		auto cache_it = m_project_cache.find(imported_project_info.root_path);
		if (cache_it != m_project_cache.end())
		{
			qCritical() << "Project already imported!";
			return false;
		}

		QFile project_file(path_info.absoluteFilePath());
		if (project_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open project file!";
			return false;
		}

		const QByteArray project_data = project_file.readAll();
		project_file.close();

		QJsonDocument project_document(QJsonDocument::fromJson(project_data));
		if (load_project_info(project_document, imported_project_info) == false)
		{
			return false;
		}

		CachedProjectInfo cached_info;
		cached_info.name = imported_project_info.name;
		cached_info.path = project_path;

		if (add_project_to_cache(imported_project_info.root_path, cached_info) == false)
		{
			// TODO: report error?
		}

		return true;
	}

	bool ProjectManager::load_project(const QString& project_path)
	{
		if (is_project_loaded() == true)
		{
			qCritical() << "Project already loaded!";
			return false;
		}

		const QFileInfo project_file_info(project_path);
		if (internal_load_project(project_file_info, m_loaded_project_info) == false)
		{
			return false;
		}

		emit project_loaded();

		return true;
	}

	void ProjectManager::remove_project(const QString& project_path)
	{
		if (is_project_loaded() == true)
		{
			qCritical() << "Cannot remove projects while a project is already loaded!";
			return;
		}

		QFileInfo project_file_info(project_path);
		if (validate_project_file(project_file_info) == false)
		{
			qCritical() << "Project path not valid!";
			return;
		}

		auto cache_it = m_project_cache.find(project_file_info.absolutePath());
		if (cache_it != m_project_cache.end())
		{
			m_project_cache.erase(cache_it);
		}
		save_project_cache();
	}

	QList<EditorPluginInfo> ProjectManager::find_editor_plugins(const QString& search_path) const
	{
		QList<EditorPluginInfo> plugin_entries;
		QDirIterator dir_iterator(search_path, QStringList() << QString("*.%1").arg(c_editor_plugin_suffix), QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (dir_iterator.hasNext())
		{
			QFile current_file(dir_iterator.next());
			if (current_file.open(QIODevice::ReadOnly) == false)
			{
				qCritical() << "Failed to open editor plugin file" << dir_iterator.filePath();
				continue;
			}

			const QByteArray editor_plugin_data = current_file.readAll();
			current_file.close();

			EditorPluginInfo plugin_info;
			QJsonDocument editor_plugin_document(QJsonDocument::fromJson(editor_plugin_data));
			if (load_editor_plugin_info(editor_plugin_document, plugin_info) == false)
			{
				qCritical() << "Failed to load editor plugin data from" << dir_iterator.filePath();
				continue;
			}

			const QFileInfo plugin_file_info = dir_iterator.fileInfo();
			
			// NOTE: we can get the plugin file name and extension by just trimming the import file suffix
			plugin_info.path = QDir::cleanPath(plugin_file_info.absolutePath() + "/" + plugin_file_info.completeBaseName());

			plugin_entries.push_back(plugin_info);
		}

		return plugin_entries;
	}

	QList<GameExecutableInfo> ProjectManager::find_game_executables(const QString& search_path) const
	{
		QList<GameExecutableInfo> executable_entries;
		QDirIterator dir_iterator(search_path, QStringList() << QString("*.%1").arg(c_game_executable_suffix), QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (dir_iterator.hasNext())
		{
			QFile current_file(dir_iterator.next());
			if (current_file.open(QIODevice::ReadOnly) == false)
			{
				qCritical() << "Failed to open game executable file" << dir_iterator.filePath();
				continue;
			}

			const QByteArray game_executable_data = current_file.readAll();
			current_file.close();

			GameExecutableInfo executable_info;
			QJsonDocument game_executable_document(QJsonDocument::fromJson(game_executable_data));
			if (load_game_executable_info(game_executable_document, executable_info) == false)
			{
				qCritical() << "Failed to load game executable data from" << dir_iterator.filePath();
				continue;
			}

			const QFileInfo executable_file_info = dir_iterator.fileInfo();

			// NOTE: we can get the executable file name and extension by just trimming the import file suffix
			executable_info.path = QDir::cleanPath(executable_file_info.absolutePath() + "/" + executable_file_info.completeBaseName());

			executable_entries.push_back(executable_info);
		}

		return executable_entries;
	}

	ProjectManager::ProjectManager(Application& application)
		: m_application(application)
	{

	}

	bool ProjectManager::initialize()
	{
		if (load_project_cache() == false)
		{
			return false;
		}

		return true;
	}

	void ProjectManager::shutdown()
	{
		// TODO: anything?
	}

	bool ProjectManager::load_project_cache()
	{
		m_project_cache.clear();

		QSettings app_settings = Application::get_app_settings();

		int project_count = app_settings.beginReadArray(c_project_cache_prefix);
		for (int project_index = 0; project_index < project_count; ++project_index)
		{
			app_settings.setArrayIndex(project_index);

			const QString project_path = app_settings.value(get_app_settings_key(ApplicationSetting::PATH)).toString();
			const QString root_path = QFileInfo(project_path).absolutePath();
			if (m_project_cache.contains(root_path) == true)
			{
				qCritical() << "Duplicate project UUID!";
				continue;
			}

			CachedProjectInfo cached_info;
			cached_info.name = app_settings.value(get_app_settings_key(ApplicationSetting::NAME)).toString();
			cached_info.path = project_path;
			if (app_settings.contains(get_editor_plugin_settings_key(ProjectEditorPluginSetting::CUSTOM_SEARCH_PATH)) == true)
			{
				cached_info.plugin_settings.custom_search_path = app_settings.value(get_editor_plugin_settings_key(ProjectEditorPluginSetting::CUSTOM_SEARCH_PATH)).toString();
			}
			if (app_settings.contains(get_editor_plugin_settings_key(ProjectEditorPluginSetting::SELECTED_CONFIGURATION)) == true)
			{
				cached_info.plugin_settings.selected_config = app_settings.value(get_editor_plugin_settings_key(ProjectEditorPluginSetting::SELECTED_CONFIGURATION)).toString();
			}

			if (app_settings.contains(get_game_executable_settings_key(ProjectGameExecutableSetting::CUSTOM_SEARCH_PATH)) == true)
			{
				cached_info.game_settings.custom_search_path = app_settings.value(get_game_executable_settings_key(ProjectGameExecutableSetting::CUSTOM_SEARCH_PATH)).toString();
			}
			if (app_settings.contains(get_game_executable_settings_key(ProjectGameExecutableSetting::SELECTED_CONFIGURATION)) == true)
			{
				cached_info.game_settings.selected_config = app_settings.value(get_game_executable_settings_key(ProjectGameExecutableSetting::SELECTED_CONFIGURATION)).toString();
			}

			m_project_cache[root_path] = cached_info;
		}
		app_settings.endArray();

		return true;
	}

	bool ProjectManager::save_project_cache() const
	{
		QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, Application::c_org_name, Application::c_app_name);

		settings.beginWriteArray(c_project_cache_prefix, m_project_cache.size());
		int array_index = 0;
		for (auto cache_it = m_project_cache.begin(); cache_it != m_project_cache.end(); ++cache_it)
		{
			settings.setArrayIndex(array_index);

			const CachedProjectInfo& current_proj_info = cache_it.value();

			settings.setValue("name", current_proj_info.name);
			settings.setValue("path", current_proj_info.path);

			if (current_proj_info.plugin_settings.custom_search_path.isEmpty() == false)
			{
				settings.setValue(get_editor_plugin_settings_key(ProjectEditorPluginSetting::CUSTOM_SEARCH_PATH), current_proj_info.plugin_settings.custom_search_path);
			}
			else
			{
				settings.remove(get_editor_plugin_settings_key(ProjectEditorPluginSetting::CUSTOM_SEARCH_PATH));
			}

			if (current_proj_info.plugin_settings.selected_config.isEmpty() == false)
			{
				settings.setValue(get_editor_plugin_settings_key(ProjectEditorPluginSetting::SELECTED_CONFIGURATION), current_proj_info.plugin_settings.selected_config);
			}
			else
			{
				settings.remove(get_editor_plugin_settings_key(ProjectEditorPluginSetting::SELECTED_CONFIGURATION));
			}

			if (current_proj_info.game_settings.custom_search_path.isEmpty() == false)
			{
				settings.setValue(get_game_executable_settings_key(ProjectGameExecutableSetting::CUSTOM_SEARCH_PATH), current_proj_info.game_settings.custom_search_path);
			}
			else
			{
				settings.remove(get_game_executable_settings_key(ProjectGameExecutableSetting::CUSTOM_SEARCH_PATH));
			}

			if (current_proj_info.game_settings.selected_config.isEmpty() == false)
			{
				settings.setValue(get_game_executable_settings_key(ProjectGameExecutableSetting::SELECTED_CONFIGURATION), current_proj_info.game_settings.selected_config);
			}
			else
			{
				settings.remove(get_game_executable_settings_key(ProjectGameExecutableSetting::SELECTED_CONFIGURATION));
			}

			++array_index;
		}
		settings.endArray();
		settings.sync();

		return true;
	}

	bool ProjectManager::internal_load_project(const QFileInfo& project_file_info, ProjectInfo& project_info)
	{
		if (validate_project_file(project_file_info) == false)
		{
			return false;
		}

		QFile project_file(project_file_info.absoluteFilePath());
		if (project_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open project file!";
			return false;
		}

		const QByteArray project_data = project_file.readAll();
		project_file.close();

		QJsonDocument project_document(QJsonDocument::fromJson(project_data));
		if (load_project_info(project_document, project_info) == false)
		{
			return false;
		}

		project_info.root_path = project_file_info.absolutePath();

		// NOTE: only load data schema in Editor mode
		if (m_application.get_configuration().mode == ApplicationMode::EDITOR)
		{
			load_project_data_schema();
		}

		auto cached_project_it = m_project_cache.find(project_info.root_path);
		if (cached_project_it != m_project_cache.end())
		{
			// Add metadata from cache
			project_info.plugin_settings.custom_search_path = cached_project_it->plugin_settings.custom_search_path;
			project_info.plugin_settings.selected_configuration = cached_project_it->plugin_settings.selected_config;

			project_info.game_settings.custom_search_path = cached_project_it->game_settings.custom_search_path;
			project_info.game_settings.selected_configuration = cached_project_it->game_settings.selected_config;
		}
		else
		{
			// Add to cache
			CachedProjectInfo cached_info;
			cached_info.name = project_info.name;
			cached_info.path = project_file_info.absoluteFilePath();

			if (add_project_to_cache(project_info.root_path, cached_info) == false)
			{
				// TODO: log error?
			}
		}

		// Load plugins and game executables
		{
			const QString search_path = project_info.plugin_settings.custom_search_path.isEmpty() ? project_info.root_path : project_info.plugin_settings.custom_search_path;
			project_info.plugin_entries = find_editor_plugins(search_path);

			if (project_info.plugin_settings.selected_configuration.isEmpty() == false)
			{
				bool cached_config_found = false;
				for (const EditorPluginInfo& current_entry : project_info.plugin_entries)
				{
					if (current_entry.configuration_name == project_info.plugin_settings.selected_configuration)
					{
						cached_config_found = true;
						break;
					}
				}
				if (cached_config_found == false)
				{
					qWarning() << "Cannot find editor plugin configuration" << project_info.plugin_settings.selected_configuration << "among available plugins!";
				}
			}
		}

		{
			const QString search_path = project_info.game_settings.custom_search_path.isEmpty() ? project_info.root_path : project_info.game_settings.custom_search_path;
			project_info.game_entries = find_game_executables(search_path);

			if (project_info.game_settings.selected_configuration.isEmpty() == false)
			{
				bool cached_config_found = false;
				for (const GameExecutableInfo& current_entry : project_info.game_entries)
				{
					if (current_entry.configuration_name == project_info.game_settings.selected_configuration)
					{
						cached_config_found = true;
						break;
					}
				}
				if (cached_config_found == false)
				{
					qWarning() << "Cannot find game executable configuration" << project_info.game_settings.selected_configuration << "among available executables!";
				}
			}
		}

		// TODO: anything else?
		return true;
	}

	bool ProjectManager::internal_save_project(const QFileInfo& project_file_info, const ProjectInfo& project_info)
	{
		// Make sure path exists
		const QDir project_dir;
		if (project_dir.mkpath(project_file_info.absolutePath()) == false)
		{
			qCritical() << "Failed to create project directory!";
			return false;
		}

		QFile project_file(project_file_info.absoluteFilePath());
		if (project_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to write project file!";
			return false;
		}

		const QUuid project_name_property_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);
		const QUuid custom_data_resource_id = Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_resource_property.id);

		QJsonObject project_data_root;
		project_data_root[Utilities::serialize_labeled_uuid(L"name", project_name_property_id)] = project_info.name;
		if (Utilities::is_uuid_valid(project_info.custom_data_resource_id) == true)
		{
			project_data_root[Utilities::serialize_labeled_uuid(L"custom_data_resource", custom_data_resource_id)] = Utilities::uuid_to_base64_string(project_info.custom_data_resource_id);
		}

		project_file.write(QJsonDocument(project_data_root).toJson());
		project_file.close();

		return true;
	}

	bool ProjectManager::add_project_to_cache(const QString& root_path, const CachedProjectInfo& cached_info)
	{
		m_project_cache[root_path] = cached_info;
		return save_project_cache();
	}

	void ProjectManager::save_current_project_data()
	{
		auto project_it = m_project_cache.find(m_loaded_project_info.root_path);
		if (project_it == m_project_cache.end())
		{
			qCritical() << "Project not in cache!";
			return;
		}

		internal_save_project(QFileInfo(project_it->path), m_loaded_project_info);
	}
}