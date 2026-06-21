#ifndef VADONEDITOR_CORE_EDITOR_HPP
#define VADONEDITOR_CORE_EDITOR_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <memory>
#include <string>
namespace Vadon::Core
{
	class EngineCoreInterface;
	class EngineEnvironment;
}
namespace Vadon::Utilities
{
	class CommandLineParser;
}
namespace VadonEditor::Model
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
		VADONEDITOR_API Editor(Vadon::Core::EngineCoreInterface& engine_core);
		VADONEDITOR_API ~Editor();

		VADONEDITOR_API static void init_environment(Vadon::Core::EngineEnvironment& environment);

		VADONEDITOR_API bool initialize();
		VADONEDITOR_API bool load_project(std::string_view root_path);
		VADONEDITOR_API void shutdown();

		VADONEDITOR_API Vadon::Core::EngineCoreInterface& get_engine_core();

		VADONEDITOR_API Vadon::Utilities::CommandLineParser& get_command_line_parser();

		VADONEDITOR_API MetadataRegistry& get_metadata_registry();
		VADONEDITOR_API const MetadataRegistry& get_metadata_registry() const;

		VADONEDITOR_API ProjectManager& get_project_manager();

		VADONEDITOR_API Model::ResourceSystem& get_resource_system();
		VADONEDITOR_API Model::SceneSystem& get_scene_system();

		VADONEDITOR_API void process_message(const char* data, size_t size);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif