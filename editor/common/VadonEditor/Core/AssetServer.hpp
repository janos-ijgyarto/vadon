#ifndef VADONEDITOR_CORE_ASSETSERVER_HPP
#define VADONEDITOR_CORE_ASSETSERVER_HPP
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
	class ResourceDatabase;
}
namespace VadonEditor::Core
{
	class ProjectManager;

	class AssetServer
	{
	public:
		VADONEDITOR_API AssetServer(Vadon::Core::EngineCoreInterface& engine_core);
		VADONEDITOR_API ~AssetServer();

		VADONEDITOR_API static void init_environment(Vadon::Core::EngineEnvironment& environment);

		VADONEDITOR_API bool initialize();
		VADONEDITOR_API bool load_project(std::string_view root_path);
		VADONEDITOR_API void shutdown();

		VADONEDITOR_API Vadon::Core::EngineCoreInterface& get_engine_core();

		VADONEDITOR_API Vadon::Utilities::CommandLineParser& get_command_line_parser();

		VADONEDITOR_API ProjectManager& get_project_manager();

		VADONEDITOR_API Model::ResourceDatabase& get_resource_database();

		VADONEDITOR_API void process_message(const char* data, size_t size);
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif