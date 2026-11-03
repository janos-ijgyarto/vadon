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
		struct ProjectCacheEntry
		{
			QString path;
		};
		using ProjectCacheEntryList = QList<ProjectCacheEntry>;

		const SourceProject& get_source_project() const { return m_source_project; }
		const EditorProject& get_editor_project() const { return m_editor_project; }

		// NOTE: only updates editor metadata, base info is immutable!
		void update_project_info(const SourceProject& source_project, const EditorProject& editor_project);

		bool is_project_loaded() const { return m_source_project.is_valid() && m_editor_project.is_valid(); }

		const DataSchema& get_project_data_schema() const { return m_loaded_project_schema; }

		bool generate_project_data_schema(const QString& plugin_config);
		bool load_project_data_schema();
		
		const ProjectCacheEntryList get_cached_project_list() const;

		// NOTE: create assumes a new unique project is created, import assumes it already exists
		bool create_project(const QString& name, const EditorProjectInfo& project_info);
		bool import_project(const EditorProjectInfo& project_info);
		bool load_project(const QString& editor_project_path);
		void remove_project(const QString& editor_project_path);

		static SourceProjectInfo load_source_project_info(const QString& project_path);
		static EditorProjectInfo load_editor_project_info(const QString& project_path);

		static QList<EditorPluginInfo> find_editor_plugins(const QString& search_path);
		static QList<GameExecutableInfo> find_game_executables(const QString& search_path);
	signals:
		void project_loaded();
	private:
		ProjectManager(Application& application);

		bool initialize();
		void shutdown();

		bool load_project_cache();
		bool save_project_cache() const;
		bool add_project_to_cache(const EditorProjectInfo& project_info);

		bool internal_save_project_data() const;

		Application& m_application;

		SourceProject m_source_project;
		EditorProject m_editor_project;
		DataSchema m_loaded_project_schema;

		// NOTE: using this for more convenient lookup, serialization is done via QSettings!
		QHash<QString, ProjectCacheEntry> m_project_cache;

		friend Application;
	};
}
#endif