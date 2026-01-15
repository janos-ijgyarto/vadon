#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/CommandLine.hpp>

#include <QDir>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QSettings>

namespace
{
	constexpr const char* c_project_cache_prefix = "ProjectManager/project_cache/projects";

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
		return validate_project_name(project_info.name) && validate_project_plugin_path(project_info.plugin_path);
	}

	bool load_project_info(const QJsonDocument& json_doc, VadonEditor::Core::ProjectInfo& project_info)
	{
		if (json_doc.isNull() == true)
		{
			// TODO: more detailed error!
			qCritical() << "Project file contains invalid data!";
			return false;
		}

		// TODO: validate JSON data!
		const QJsonObject cache_root = json_doc.object();
		project_info.name = cache_root["name"].toString();
		
		return true;
	}
}

namespace VadonEditor::Core
{
	const QList<ProjectManager::CachedProjectInfo> ProjectManager::get_cached_project_list() const
	{
		QList<ProjectManager::CachedProjectInfo> project_list;
		for (auto cache_it = m_project_cache.begin(); cache_it != m_project_cache.end(); ++cache_it)
		{
			project_list.append(cache_it.value());
		}

		return project_list;
	}

	bool ProjectManager::set_project_name(const QString& name)
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::set_project_name", "Project not loaded");

		if (validate_project_name(name) == false)
		{
			qCritical() << "Invalid project name!";
			return false;
		}

		m_loaded_project_info.name = name;

		save_current_project_data();
		save_project_cache();
		return true;
	}

	bool ProjectManager::set_plugin_path(const QString& path)
	{
		Q_ASSERT_X(is_project_loaded() == true, "ProjectManager::set_plugin_path", "Project not loaded");
		
		if (validate_project_plugin_path(path) == false)
		{
			qCritical() << "Invalid plugin path!";
			return false;
		}

		m_loaded_project_info.plugin_path = path;

		auto project_it = m_project_cache.find(m_loaded_project_info.root_path);
		if (project_it != m_project_cache.end())
		{
			project_it->plugin_path = path;
		}
		else
		{
			qCritical() << "Project not in cache!";
			return false;
		}

		save_project_cache();
		return true;
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
		cached_info.name = m_loaded_project_info.name;
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

		QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, Application::c_org_name, Application::c_app_name);

		int project_count = settings.beginReadArray(c_project_cache_prefix);
		for (int project_index = 0; project_index < project_count; ++project_index)
		{
			settings.setArrayIndex(project_index);

			const QString project_path = settings.value("path").toString();
			const QString root_path = QFileInfo(project_path).absolutePath();
			if (m_project_cache.contains(root_path) == true)
			{
				qCritical() << "Duplicate project UUID!";
				continue;
			}

			CachedProjectInfo cached_info;
			cached_info.name = settings.value("name").toString();
			cached_info.path = project_path;
			if (settings.contains("plugin_path") == true)
			{
				cached_info.plugin_path = settings.value("plugin_path").toString();
			}

			m_project_cache[root_path] = cached_info;
		}
		settings.endArray();

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

			if (current_proj_info.plugin_path.isEmpty() == false)
			{
				settings.setValue("plugin_path", current_proj_info.plugin_path);
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

		auto cached_project_it = m_project_cache.find(project_info.root_path);
		if (cached_project_it != m_project_cache.end())
		{
			// Add metadata from cache
			project_info.plugin_path = cached_project_it->plugin_path;
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

		QJsonObject project_data_root;
		project_data_root["name"] = project_info.name;

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