#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>
#include <VadonEditor/Core/TypeInfo/MetadataRegistry.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/Utilities/System/CommandLine/Parser.hpp>

namespace VadonEditor::Core
{
	struct Editor::Internal
	{
		Vadon::Utilities::CommandLineParser m_command_line_parser;

		ProjectManager m_project_manager;
		MetadataRegistry m_metadata_registry;

		Model::ResourceSystem m_resource_system;
		Model::SceneSystem m_scene_system;
	};

	Editor::Editor()
		: m_internal(std::make_unique<Internal>())
	{
	}

	Editor::~Editor() = default;

	void Editor::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	bool Editor::initialize()
	{
		// TODO: anything?
		return true;
	}

	void Editor::shutdown()
	{
		// TODO: anything?
	}

	Vadon::Utilities::CommandLineParser& Editor::get_command_line_parser() { return m_internal->m_command_line_parser; }

	MetadataRegistry& Editor::get_metadata_registry() { return m_internal->m_metadata_registry; }

	const MetadataRegistry& Editor::get_metadata_registry() const { return m_internal->m_metadata_registry; }

	ProjectManager& Editor::get_project_manager() { return m_internal->m_project_manager; }

	Model::ResourceSystem& Editor::get_resource_system() { return m_internal->m_resource_system; }

	Model::SceneSystem& Editor::get_scene_system() { return m_internal->m_scene_system; }
}