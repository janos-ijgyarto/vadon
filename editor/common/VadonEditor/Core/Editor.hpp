#ifndef VADONEDITOR_CORE_EDITOR_HPP
#define VADONEDITOR_CORE_EDITOR_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace Vadon::Utilities
{
	class CommandLineParser;
}
namespace VadonEditor::Scene
{
	class ResourceSystem;
	class SceneSystem;
}
namespace VadonEditor::Core
{
	class MetadataRegistry;
	class ProjectManager;

	class Editor
	{
	public:
		VADONEDITOR_API Editor();
		VADONEDITOR_API ~Editor();

		VADONEDITOR_API static void init_environment(Vadon::Core::EngineEnvironment& environment);

		VADONEDITOR_API bool initialize();
		VADONEDITOR_API void shutdown();

		VADONEDITOR_API Vadon::Utilities::CommandLineParser& get_command_line_parser();

		VADONEDITOR_API MetadataRegistry& get_metadata_registry();
		VADONEDITOR_API const MetadataRegistry& get_metadata_registry() const;

		VADONEDITOR_API ProjectManager& get_project_manager();

		VADONEDITOR_API Scene::ResourceSystem& get_resource_system();
		VADONEDITOR_API Scene::SceneSystem& get_scene_system();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif