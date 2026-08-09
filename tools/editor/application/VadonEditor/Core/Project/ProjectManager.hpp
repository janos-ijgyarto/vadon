#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/Core/Project/Project.hpp>
#include <VadonEditor/Core/Data/Schema.hpp>
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
		struct CachedProjectEditorPluginSettings
		{
			QString custom_search_path;
			QString selected_config;
		};

		struct CachedProjectGameSettings
		{
			QString custom_search_path;
			QString selected_config;
		};

		struct CachedProjectInfo
		{
			QString name;
			QString path;
			CachedProjectEditorPluginSettings plugin_settings;
			CachedProjectGameSettings game_settings;
		};

		const ProjectInfo& get_project_info() const { return m_loaded_project_info; }
		void set_project_info(const ProjectInfo& project_info);

		bool is_project_loaded() const { return m_loaded_project_info.name.isEmpty() == false; }

		const DataSchema& get_project_data_schema() const { return m_loaded_project_schema; }

		bool generate_project_data_schema(const QString& plugin_config);
		bool load_project_data_schema();
		
		const QList<CachedProjectInfo> get_cached_project_list() const;

		bool create_project(const ProjectInfo& project_info);
		bool import_project(const QString& project_path);
		bool load_project(const QString& project_path);
		void remove_project(const QString& project_path);

		QList<EditorPluginInfo> find_editor_plugins(const QString& search_path) const;
		QList<GameExecutableInfo> find_game_executables(const QString& search_path) const;
	signals:
		void project_loaded();
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
		DataSchema m_loaded_project_schema;

		// NOTE: using this for more convenient lookup, serialization is done via QSettings!
		QHash<QString, CachedProjectInfo> m_project_cache;

		friend Application;
	};
}
#endif