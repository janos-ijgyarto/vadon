#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

namespace VadonEditor::Core
{
	struct Editor::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;
		Vadon::Utilities::CommandLineParser m_command_line_parser;

		Vadon::ECS::World m_ecs_world;

		ProjectManager m_project_manager;
		MetadataRegistry m_metadata_registry;

		Model::ResourceSystem m_resource_system;
		Model::SceneSystem m_scene_system;

		Internal(Editor& editor, Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
			, m_resource_system(editor, engine_core, m_project_manager)
			, m_scene_system(editor)
		{

		}
	};

	Editor::Editor(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(*this, engine_core))
	{
	}

	Editor::~Editor() = default;

	void Editor::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	bool Editor::initialize()
	{
		if (get_resource_system().initialize() == false)
		{
			return false;
		}

		return true;
	}

	bool Editor::load_project(std::string_view root_path)
	{
		if (get_project_manager().load_project(get_engine_core(), root_path) == false)
		{
			return false;
		}

		if (get_resource_system().project_loaded() == false)
		{
			return false;
		}

		return true;
	}

	void Editor::shutdown()
	{
		m_internal->m_scene_system.shutdown();
		m_internal->m_resource_system.shutdown();
	}

	Vadon::Core::EngineCoreInterface& Editor::get_engine_core() { return m_internal->m_engine_core; }

	Vadon::ECS::World& Editor::get_ecs_world() { return m_internal->m_ecs_world; }

	Vadon::Utilities::CommandLineParser& Editor::get_command_line_parser() { return m_internal->m_command_line_parser; }

	MetadataRegistry& Editor::get_metadata_registry() { return m_internal->m_metadata_registry; }

	const MetadataRegistry& Editor::get_metadata_registry() const { return m_internal->m_metadata_registry; }

	ProjectManager& Editor::get_project_manager() { return m_internal->m_project_manager; }

	Model::ResourceSystem& Editor::get_resource_system() { return m_internal->m_resource_system; }

	Model::SceneSystem& Editor::get_scene_system() { return m_internal->m_scene_system; }

	void Editor::process_message(const char* data, size_t size)
	{
		get_resource_system().process_message(data, size);
		get_scene_system().process_message(data, size);
	}
}