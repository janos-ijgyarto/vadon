#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/Core/Module.hpp>
#include <VadonEditor/Core/Project/Project.hpp>
#include <VadonEditor/Core/Project/Asset/AssetLibrary.hpp>
namespace VadonEditor::Core
{
	class ProjectManager : public CoreSystem<ProjectManager>
	{
	public:
		enum class State
		{
			LAUNCHER,
			PROJECT_LOADED,
			PROJECT_OPEN,
			PROJECT_CLOSED
		};

		State get_state() const { return m_state; }

		const ProjectInfoList& get_project_cache() const { return m_project_cache; }
		
		const Project& get_active_project() const { return m_active_project; }

		bool create_project(std::string_view project_name, std::string_view root_path); // FIXME: provide other params!
		bool open_project(std::string_view path);
		void close_project();

		AssetLibrary& get_asset_library() { return m_asset_library; }
	private:
		ProjectManager(Editor& editor);

		bool initialize();
		void load_project_cache();
		bool open_startup_project();

		bool load_project(std::string_view root_path);
		void add_project_to_cache(const ProjectInfo& project);

		State m_state;
		ProjectInfoList m_project_cache;
		Project m_active_project;

		AssetLibrary m_asset_library;

		friend Editor;
	};
}
#endif