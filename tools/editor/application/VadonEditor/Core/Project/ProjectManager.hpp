#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/Core/Project/Project.hpp>
#include <QObject>
#include <QHash>
class QFileInfo;
class QJsonDocument;
namespace VadonEditor::Core
{
	class Application;

	class ProjectManager : public QObject
	{
		Q_OBJECT
	public:
		struct CachedProjectInfo
		{
			QString name;
			QString path;
			QString plugin_path;
		};

		const ProjectInfo& get_project_info() const { return m_loaded_project_info; }
		bool is_project_loaded() const { return m_loaded_project_info.name.isEmpty() == false; }
		
		const QList<CachedProjectInfo> get_cached_project_list() const;

		// TODO: have some kind of attribute system?
		bool set_project_name(const QString& name);
		bool set_plugin_path(const QString& path);

		bool create_project(const VadonEditor::Core::ProjectInfo& project_info);
		bool import_project(const QString& project_path);
		bool load_project(const QString& project_path);
		void remove_project(const QString& project_path);
	signals:
		void project_loaded();
		void project_plugin_path_modified();
	private:
		ProjectManager(Application& application);

		bool initialize();
		void shutdown();

		bool load_project_cache();
		bool save_project_cache() const;

		bool internal_load_project(const QFileInfo& project_file_info, ProjectInfo& project_info);
		bool internal_save_project(const QFileInfo& project_file_info, const ProjectInfo& project_info);

		bool add_project_to_cache(const QString& root_path, const CachedProjectInfo& cached_info);
		void save_current_project_data();

		Application& m_application;

		ProjectInfo m_loaded_project_info;

		// NOTE: using this for more convenient lookup, serialization is done via QSettings!
		QHash<QString, CachedProjectInfo> m_project_cache;

		friend Application;
	};
}
#endif