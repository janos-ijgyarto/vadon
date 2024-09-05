#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/Core/Module.hpp>
#include <VadonEditor/Core/Project/Project.hpp>
namespace VadonEditor::Core
{
	class ProjectManager : public CoreSystem<ProjectManager>
	{
	public:
		enum class State
		{
			LAUNCHER,
			PROJECT_OPENED,
			PROJECT_ACTIVE,
			PROJECT_CLOSED
		};

		State get_state() const { return m_state; }

		const ProjectList& get_project_cache() const { return m_project_cache; }
		
		const Project& get_active_project() const { return m_active_project; }

		bool create_project(std::string_view name, std::string_view path); // FIXME: provide other params!
		bool open_project(std::string_view path);
		void close_project();
	private:
		ProjectManager(Editor& editor);

		bool initialize();
		bool open_startup_project();

		bool load_project(std::string_view root_path);

		State m_state;
		ProjectList m_project_cache;
		Project m_active_project;

		friend Editor;
	};
}
#endif