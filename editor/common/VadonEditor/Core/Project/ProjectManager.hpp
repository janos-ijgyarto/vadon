#ifndef VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#define VADONEDITOR_CORE_PROJECT_PROJECTMANAGER_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <Vadon/Core/Project/Project.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
}
namespace VadonEditor::Core
{
	class ProjectManager
	{
	public:
		bool is_project_loaded() const { return m_active_project.root_path.empty() == false; }

		const Vadon::Core::Project& get_active_project() const { return m_active_project; }
	private:
		ProjectManager();

		bool load_project(Vadon::Core::EngineCoreInterface& engine_core, std::string_view root_path);

		Vadon::Core::Project m_active_project;

		// FIXME: find a better way to share between the two classes?
		friend class AssetServer;
		friend class Editor;
	};
}
#endif