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
	constexpr const char* c_project_manager_setting_prefix = "ProjectManager";
	constexpr const char* c_editor_plugin_suffix = "vdeplugin";
	constexpr const char* c_game_executable_suffix = "vdgexe";

	enum class ProjectManagerSetting
	{
		PROJECT_CACHE,
		SETTINGS_COUNT
	};

	constexpr const char* c_project_manager_settings_keys[static_cast<size_t>(ProjectManagerSetting::SETTINGS_COUNT)] = {
		"project_cache"
	};

	constexpr const char* get_project_manager_settings_key(ProjectManagerSetting setting) { return c_project_manager_settings_keys[static_cast<size_t>(setting)]; }

	enum class ProjectCacheSetting
	{
		PROJECTS,
		SETTINGS_COUNT
	};

	constexpr const char* c_project_cache_settings_keys[static_cast<size_t>(ProjectCacheSetting::SETTINGS_COUNT)] = {
		"projects"
	};

	constexpr const char* get_project_cache_settings_key(ProjectCacheSetting setting) { return c_project_cache_settings_keys[static_cast<size_t>(setting)]; }

	enum class ProjectCacheEntrySetting
	{
		PATH,
		SETTINGS_COUNT
	};

	constexpr const char* c_project_cache_entry_settings_keys[static_cast<size_t>(ProjectCacheEntrySetting::SETTINGS_COUNT)] = {
		"path"
	};

	constexpr const char* get_project_cache_entry_settings_key(ProjectCacheEntrySetting setting) { return c_project_cache_entry_settings_keys[static_cast<size_t>(setting)]; }

	bool validate_project_name(const QString& name)
	{
		if (name.isEmpty() == true)
		{
			return false;
		}

		// TODO: illegal characters, etc?
		return true;
	}

	bool validate_source_project_file(const QFileInfo& project_file)
	{
		if (project_file.exists() == false)
		{
			return false;
		}

		if (project_file.isFile() == false)
		{
			return false;
		}

		if (project_file.fileName() != VadonEditor::Core::SourceProjectInfo::c_project_file_name)
		{
			return false;
		}

		return true;
	}

	bool validate_editor_project_file(const QFileInfo& project_file)
	{
		if (project_file.exists() == false)
		{
			return false;
		}

		if (project_file.isFile() == false)
		{
			return false;
		}

		if (project_file.fileName() != VadonEditor::Core::EditorProjectInfo::c_file_name)
		{
			return false;
		}

		return true;
	}

	bool load_source_project_info(const QJsonDocument& json_doc, VadonEditor::Core::SourceProjectInfo& project_info)
	{
		if (json_doc.isNull() == true)
		{
			// TODO: more detailed error!
			qCritical() << "Source project file contains invalid data!";
			return false;
		}

		const QUuid project_name_property_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);

		// TODO: validate JSON data!
		const QJsonObject project_info_root = json_doc.object();

		for (auto property_it = project_info_root.begin(); property_it != project_info_root.end(); ++property_it)
		{
			const QUuid property_uuid = VadonEditor::Utilities::parse_labeled_uuid(property_it.key());
			if (property_uuid == project_name_property_id)
			{
				project_info.name = property_it.value().toString();
			}
		}

		return true;
	}

	bool load_source_project_file(VadonEditor::Core::SourceProject& source_project)
	{
		QFileInfo project_file_info(source_project.info.get_project_file_path());

		QFile project_file(project_file_info.absoluteFilePath());
		if (project_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open editor project file!";
			return false;
		}

		const QByteArray project_file_buffer = project_file.readAll();
		project_file.close();

		QJsonDocument project_document(QJsonDocument::fromJson(project_file_buffer));

		if (load_source_project_info(project_document, source_project.info) == false)
		{
			return false;
		}

		const QUuid custom_data_resource_property_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_resource_property.id);

		// TODO: validate JSON data!
		const QJsonObject project_info_root = project_document.object();

		for (auto property_it = project_info_root.begin(); property_it != project_info_root.end(); ++property_it)
		{
			const QUuid property_uuid = VadonEditor::Utilities::parse_labeled_uuid(property_it.key());
			if (property_uuid == custom_data_resource_property_id)
			{
				source_project.custom_data_resource_id = VadonEditor::Utilities::base64_string_to_uuid(property_it.value().toString());
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

	QString format_command_line_argument_key(const QString& key)
	{
		return QString("--%1").arg(key);
	}

	QString get_editor_project_data_schema_path(const VadonEditor::Core::EditorProjectInfo& project_info)
	{
		return QDir::cleanPath(project_info.output_path + "/metadata/data_schema.json");
	}

	bool save_editor_project_data(const VadonEditor::Core::EditorProject& project)
	{
		QFileInfo project_file_info(project.info.get_project_file_path());

		// Make sure path exists
		const QDir project_dir;
		if (project_dir.mkpath(project_file_info.absolutePath()) == false)
		{
			qCritical() << "Failed to create editor project directory!";
			return false;
		}

		QFile project_file(project_file_info.absoluteFilePath());
		if (project_file.open(QIODevice::WriteOnly) == false)
		{
			qCritical() << "Failed to write editor project file!";
			return false;
		}

		QJsonObject project_root_obj;

		{
			QJsonObject project_info_obj;
			project_info_obj["source_path"] = project.info.source_path;

			project_root_obj["info"] = project_info_obj;
		}
		{
			QJsonObject plugin_settings_obj;
			plugin_settings_obj["binaries_path"] = project.plugin_settings.binaries_path;
			plugin_settings_obj["selected_config"] = project.plugin_settings.selected_configuration;

			project_root_obj["plugin_settings"] = plugin_settings_obj;
		}
		{
			QJsonObject game_settings_obj;
			game_settings_obj["binaries_path"] = project.game_settings.binaries_path;
			game_settings_obj["selected_config"] = project.game_settings.selected_configuration;

			project_root_obj["game_settings"] = game_settings_obj;
		}

		project_file.write(QJsonDocument(project_root_obj).toJson());
		project_file.close();

		return true;
	}

	bool load_editor_project_data(VadonEditor::Core::EditorProject& project)
	{
		QFileInfo project_file_info(project.info.get_project_file_path());

		QFile project_file(project_file_info.absoluteFilePath());
		if (project_file.open(QIODevice::ReadOnly) == false)
		{
			qCritical() << "Failed to open editor project file!";
			return false;
		}

		const QByteArray project_file_buffer = project_file.readAll();
		project_file.close();

		QJsonDocument project_document(QJsonDocument::fromJson(project_file_buffer));
		const QJsonObject project_root_obj = project_document.object();

		if (const QJsonValue info_value = project_root_obj["info"]; info_value.isObject())
		{
			const QJsonObject info_obj = info_value.toObject();

			if (const QJsonValue source_path_value = info_obj["source_path"]; source_path_value.isString())
			{
				project.info.source_path = source_path_value.toString();
			}
		}

		if (const QJsonValue plugin_settings_value = project_root_obj["plugin_settings"]; plugin_settings_value.isObject())
		{
			const QJsonObject plugin_settings_obj = plugin_settings_value.toObject();

			if (const QJsonValue binaries_path_value = plugin_settings_obj["binaries_path"]; binaries_path_value.isString())
			{
				project.plugin_settings.binaries_path = binaries_path_value.toString();
			}

			if (const QJsonValue selected_config_value = plugin_settings_obj["selected_config"]; selected_config_value.isString())
			{
				project.plugin_settings.selected_configuration = selected_config_value.toString();
			}
		}

		if (const QJsonValue game_settings_value = project_root_obj["game_settings"]; game_settings_value.isObject())
		{
			const QJsonObject game_settings_obj = game_settings_value.toObject();

			if (const QJsonValue binaries_path_value = game_settings_obj["binaries_path"]; binaries_path_value.isString())
			{
				project.game_settings.binaries_path = binaries_path_value.toString();
			}

			if (const QJsonValue selected_config_value = game_settings_obj["selected_config"]; selected_config_value.isString())
			{
				project.game_settings.selected_configuration = selected_config_value.toString();
			}
		}

		// TODO: anything else?
		return true;
	}

	bool save_source_project_data(const VadonEditor::Core::SourceProject& project)
	{
		const QFileInfo project_file_info(project.info.get_project_file_path());

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

		const QUuid project_name_property_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_name_property.id);
		const QUuid custom_data_resource_id = VadonEditor::Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::ProjectInfoSchema::c_custom_data_resource_property.id);

		QJsonObject project_data_root;
		project_data_root[VadonEditor::Utilities::serialize_labeled_uuid(L"name", project_name_property_id)] = project.info.name;
		if (VadonEditor::Utilities::is_uuid_valid(project.custom_data_resource_id) == true)
		{
			project_data_root[VadonEditor::Utilities::serialize_labeled_uuid(L"custom_data_resource", custom_data_resource_id)] = VadonEditor::Utilities::uuid_to_base64_string(project.custom_data_resource_id);
		}

		project_file.write(QJsonDocument(project_data_root).toJson());
		project_file.close();

		return true;
	}

	bool load_editor_project_metadata(VadonEditor::Core::EditorProject& editor_project)
	{
		// Load plugins and game executables
		{
			const QString& search_path = editor_project.get_plugin_binaries_path();
			Q_ASSERT_X(search_path.isEmpty() == false, "load_editor_project_metadata", "Search path must not be empty!");
			editor_project.plugin_entries = VadonEditor::Core::ProjectManager::find_editor_plugins(search_path);

			if (editor_project.plugin_settings.selected_configuration.isEmpty() == false)
			{
				bool cached_config_found = false;
				for (const VadonEditor::Core::EditorPluginInfo& current_entry : editor_project.plugin_entries)
				{
					if (current_entry.configuration_name == editor_project.plugin_settings.selected_configuration)
					{
						cached_config_found = true;
						break;
					}
				}
				if (cached_config_found == false)
				{
					qWarning() << "Cannot find editor plugin configuration" << editor_project.plugin_settings.selected_configuration << "among available plugins!";
				}
			}
		}

		{
			const QString& search_path = editor_project.get_game_binaries_path();
			Q_ASSERT_X(search_path.isEmpty() == false, "load_editor_project_metadata", "Search path must not be empty!");
			editor_project.game_entries = VadonEditor::Core::ProjectManager::find_game_executables(search_path);

			if (editor_project.game_settings.selected_configuration.isEmpty() == false)
			{
				bool cached_config_found = false;
				for (const VadonEditor::Core::GameExecutableInfo& current_entry : editor_project.game_entries)
				{
					if (current_entry.configuration_name == editor_project.game_settings.selected_configuration)
					{
						cached_config_found = true;
						break;
					}
				}
				if (cached_config_found == false)
				{
					qWarning() << "Cannot find game executable configuration" << editor_project.game_settings.selected_configuration << "among available executables!";
				}
			}
		}

		return true;
	}

	bool validate_folder_path(const QString& path)
	{
		if (path.isEmpty() == true)
		{
			return false;
		}

		return QFileInfo(path).isDir() == true;
	}

	bool validate_source_project_info(const VadonEditor::Core::SourceProjectInfo& project_info)
	{
		if (project_info.is_valid() == false)
		{
			return false;
		}

		return validate_folder_path(project_info.root_path);
	}

	bool validate_source_project_data(const VadonEditor::Core::SourceProject& project)
	{
		return validate_source_project_info(project.info);
	}

	bool validate_editor_project_info(const VadonEditor::Core::EditorProjectInfo& project_info)
	{
		if (project_info.is_valid() == false)
		{
			return false;
		}

		return validate_folder_path(project_info.source_path) && validate_folder_path(project_info.output_path);
	}

	bool validate_editor_project_data(const VadonEditor::Core::EditorProject& project)
	{
		if (project.is_valid() == false)
		{
			return false;
		}

		return validate_editor_project_info(project.info);
	}
}

namespace VadonEditor::Core
{
	void ProjectManager::update_project_info(const SourceProject& source_project, const EditorProject& editor_project)
	{
		Q_ASSERT_X(is_project_loaded() == true, "VadonEditor::Core::ProjectManager::update_project_info", "Project not loaded");
		Q_ASSERT_X(validate_source_project_data(source_project), "VadonEditor::Core::ProjectManager::update_project_info", "Invalid source project data!");
		Q_ASSERT_X(validate_editor_project_data(editor_project), "VadonEditor::Core::ProjectManager::update_project_info", "Invalid editor project data!");

		m_source_project.custom_data_resource_id = source_project.custom_data_resource_id;

		m_editor_project.plugin_entries = editor_project.plugin_entries;
		m_editor_project.plugin_settings = editor_project.plugin_settings;

		m_editor_project.game_entries = editor_project.game_entries;
		m_editor_project.game_settings = editor_project.game_settings;

		internal_save_project_data();
	}

	bool ProjectManager::generate_project_data_schema(const QString& plugin_config)
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::generate_project_data_schema", "Project not loaded");
		const Core::EditorProject& editor_project = get_editor_project();

		const EditorPluginInfo* editor_plugin_info = editor_project.find_plugin_entry(plugin_config);
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

			QStringList arguments{ format_command_line_argument_key(CommandLineState::get_parameter_key(CommandLineParameter::IS_SCHEMA_EXPORTER)) };
			arguments.push_back(format_command_line_argument_key(CommandLineState::get_parameter_key(CommandLineParameter::STARTUP_PROJECT_PATH)));
			arguments.push_back(editor_project.info.get_project_file_path());

			arguments.push_back(format_command_line_argument_key(CommandLineState::get_parameter_key(CommandLineParameter::PLUGIN_CONFIG_NAME)));
			arguments.push_back(plugin_config);

			arguments.push_back(format_command_line_argument_key(CommandLineState::get_parameter_key(CommandLineParameter::DEBUG_BREAK_ON_INIT)));

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
				qCritical() << "Error shutting down schema exporter!";
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
		case ApplicationMode::SCHEMA_EXPORTER:
		{
			PluginManager& plugin_manager = m_application.get_plugin_manager();

			PluginInfo plugin_info;
			plugin_info.path = editor_plugin_info->path;

			PluginHandle plugin_handle = plugin_manager.load_plugin(plugin_info);
			if (plugin_handle == PluginManager::c_invalid_plugin_handle)
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

			if (m_loaded_project_schema.save_schema(get_editor_project_data_schema_path(editor_project.info)) == false)
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
		const Core::EditorProject& editor_project = get_editor_project();

		// NOTE: load into temporary object, only replace the one in system if the load was successful
		if (m_loaded_project_schema.load_schema(get_editor_project_data_schema_path(editor_project.info)) == false)
		{
			qCritical() << "Failed to load data schema!";
			return false;
		}

		return true;
	}

	const ProjectManager::ProjectCacheEntryList ProjectManager::get_cached_project_list() const
	{
		ProjectCacheEntryList project_list;
		for (auto cache_it = m_project_cache.begin(); cache_it != m_project_cache.end(); ++cache_it)
		{
			project_list.append(cache_it.value());
		}

		return project_list;
	}

	bool ProjectManager::create_project(const QString& name, const EditorProjectInfo& editor_info)
	{
		if (validate_editor_project_info(editor_info) == false)
		{
			qCritical() << "Invalid project info!";
			return false;
		}

		if (m_project_cache.find(editor_info.output_path) != m_project_cache.end())
		{
			qCritical() << "Project already in cache!";
			return false;
		}

		const QFileInfo source_project_file(editor_info.get_source_project_file_path());
		if (source_project_file.exists() && source_project_file.isFile())
		{
			qCritical() << "Source project already exists!";
			return false;
		}

		const QFileInfo editor_project_file_info(editor_info.get_project_file_path());
		if ((editor_project_file_info.exists() == true) && (editor_project_file_info.isFile() == true))
		{
			qCritical() << "Editor project already exists!";
			return false;
		}

		// Create files by saving temp project info
		SourceProject temp_source_project;
		temp_source_project.info.name = name;
		temp_source_project.info.root_path = editor_info.source_path;
		if (save_source_project_data(temp_source_project) == false)
		{
			qCritical() << "Failed to create new source project file!";
			return false;
		}

		EditorProject temp_editor_project;
		temp_editor_project.info = editor_info;
		if (save_editor_project_data(temp_editor_project) == false)
		{
			qCritical() << "Failed to create new editor project file!";
			return false;
		}

		// Add to project cache
		if(add_project_to_cache(editor_info) == false)
		{
			// TODO: report error?
		}

		return true;
	}

	bool ProjectManager::import_project(const EditorProjectInfo& project_info)
	{
		if (validate_editor_project_info(project_info) == false)
		{
			qCritical() << "Invalid project info!";
			return false;
		}

		auto cache_it = m_project_cache.find(project_info.output_path);
		if (cache_it != m_project_cache.end())
		{
			qCritical() << "Project already imported!";
			return false;
		}

		// Make sure source project file exists
		const QFileInfo source_project_file_info(project_info.get_source_project_file_path());
		if (validate_source_project_file(source_project_file_info) == false)
		{
			qCritical() << "Invalid source project file!";
			return false;
		}

		// Create the editor project at the destination, will be needed to load the project
		EditorProject temp_editor_project;
		temp_editor_project.info = project_info;
		if (save_editor_project_data(temp_editor_project) == false)
		{
			qCritical() << "Failed to create new editor project file!";
			return false;
		}

		if (add_project_to_cache(project_info) == false)
		{
			// TODO: report error?
			qCritical() << "Failed to add imported project to cache!";
			return false;
		}

		return true;
	}

	bool ProjectManager::load_project(const QString& editor_project_path)
	{
		if (is_project_loaded() == true)
		{
			qCritical() << "Project already loaded!";
			return false;
		}

		const QFileInfo editor_file_info(editor_project_path);
		if (validate_editor_project_file(editor_file_info) == false)
		{
			qCritical() << "Editor project file is invalid!";
			return false;
		}

		EditorProject temp_editor_project;
		temp_editor_project.info.output_path = editor_file_info.absolutePath();
		if (load_editor_project_data(temp_editor_project) == false)
		{
			qCritical() << "Failed to load editor project file!";
			return false;
		}

		const QFileInfo source_file_info(SourceProjectInfo::get_project_file_path(temp_editor_project.info.source_path));
		if (validate_source_project_file(source_file_info) == false)
		{
			qCritical() << "Source project file is invalid!";
			return false;
		}

		SourceProject temp_source_project;
		temp_source_project.info.root_path = source_file_info.absolutePath();
		if (load_source_project_file(temp_source_project) == false)
		{
			qCritical() << "Failed to load source project file!";
			return false;
		}

		if (load_editor_project_metadata(temp_editor_project) == false)
		{
			qCritical() << "Failed to load project metadata!";
			return false;
		}

		m_editor_project = temp_editor_project;
		m_source_project = temp_source_project;

		// NOTE: only attempt to load data schema in Editor mode
		if (m_application.get_configuration().mode == ApplicationMode::EDITOR)
		{
			load_project_data_schema();
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
		auto cache_it = m_project_cache.find(project_file_info.absolutePath());
		if (cache_it == m_project_cache.end())
		{
			qCritical() << "Project not found in cache!";
			return;
		}

		m_project_cache.erase(cache_it);
		save_project_cache();
	}

	SourceProjectInfo ProjectManager::load_source_project_info(const QString& project_path)
	{
		const QFileInfo source_file_info(project_path);
		if (validate_source_project_file(source_file_info) == false)
		{
			qCritical() << "Source project file is invalid!";
			return SourceProjectInfo{};
		}

		SourceProject temp_project;
		temp_project.info.root_path = source_file_info.absolutePath();
		if (load_source_project_file(temp_project) == false)
		{
			qCritical() << "Failed to load source project!";
			return SourceProjectInfo{};
		}

		return temp_project.info;
	}

	EditorProjectInfo ProjectManager::load_editor_project_info(const QString& project_path)
	{
		EditorProject temp_project;
		temp_project.info.output_path = project_path;
		if (load_editor_project_data(temp_project) == false)
		{
			qCritical() << "Failed to load editor project!";
			return EditorProjectInfo{};
		}

		return temp_project.info;
	}

	QList<EditorPluginInfo> ProjectManager::find_editor_plugins(const QString& search_path)
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

	QList<GameExecutableInfo> ProjectManager::find_game_executables(const QString& search_path)
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

		app_settings.beginGroup(c_project_manager_setting_prefix);
		app_settings.beginGroup(get_project_manager_settings_key(ProjectManagerSetting::PROJECT_CACHE));

		int project_count = app_settings.beginReadArray(get_project_cache_settings_key(ProjectCacheSetting::PROJECTS));
		for (int project_index = 0; project_index < project_count; ++project_index)
		{
			app_settings.setArrayIndex(project_index);

			const QString editor_project_path = app_settings.value(get_project_cache_entry_settings_key(ProjectCacheEntrySetting::PATH)).toString();
			if (m_project_cache.contains(editor_project_path) == true)
			{
				qCritical() << "Duplicate project in cache!";
				continue;
			}

			ProjectCacheEntry cached_info;
			cached_info.path = editor_project_path;

			m_project_cache[editor_project_path] = cached_info;
		}
		app_settings.endArray();
		app_settings.endGroup();
		app_settings.endGroup();

		return true;
	}

	bool ProjectManager::save_project_cache() const
	{
		QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, Application::c_org_name, Application::c_app_name);

		settings.beginGroup(c_project_manager_setting_prefix);
		settings.beginGroup(get_project_manager_settings_key(ProjectManagerSetting::PROJECT_CACHE));

		settings.beginWriteArray(get_project_cache_settings_key(ProjectCacheSetting::PROJECTS), m_project_cache.size());
		int array_index = 0;
		for (auto cache_it = m_project_cache.begin(); cache_it != m_project_cache.end(); ++cache_it)
		{
			settings.setArrayIndex(array_index);

			const ProjectCacheEntry& current_cache_entry = cache_it.value();

			settings.setValue(get_project_cache_entry_settings_key(ProjectCacheEntrySetting::PATH), current_cache_entry.path);

			++array_index;
		}
		settings.endArray();
		settings.endGroup();
		settings.endGroup();

		settings.sync();

		return true;
	}

	bool ProjectManager::add_project_to_cache(const EditorProjectInfo& project_info)
	{
		ProjectCacheEntry cache_entry;
		cache_entry.path = project_info.output_path;

		m_project_cache[project_info.output_path] = cache_entry;

		return save_project_cache();
	}

	bool ProjectManager::internal_save_project_data() const
	{
		if (save_source_project_data(m_source_project) == false)
		{
			qCritical() << "Failed to save source project!";
			return false;
		}

		if (save_editor_project_data(m_editor_project) == false)
		{
			qCritical() << "Failed to save editor project!";
			return false;
		}

		return true;
	}
}