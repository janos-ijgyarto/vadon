#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <format>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal
	{
		Vadon::ECS::World m_ecs_world;

		ResourceSystem m_resource_system;
		SceneSystem m_scene_system;

		Internal(Core::Editor& editor)
			: m_resource_system(editor)
			, m_scene_system(editor)
		{
		}

		bool initialize()
		{
			if (m_resource_system.initialize() == false)
			{
				return false;
			}

			if (m_scene_system.initialize() == false)
			{
				return false;
			}

			return true;
		}

		bool load_project(Core::Editor& editor)
		{
			Core::ProjectManager& project_manager = editor.get_system<Core::ProjectManager>();

			VADON_ASSERT(project_manager.get_state() == Core::ProjectManager::State::PROJECT_OPEN, "Project is in incorrect state!");
			if (m_resource_system.load_project_resources() == false)
			{
				Vadon::Core::Logger::log_error("Model system: unable to load project!\n");
				return false;
			}

			return true;
		}

		bool export_project(Core::Editor& editor, std::string_view output_path)
		{
			Core::ProjectManager& project_manager = editor.get_system<Core::ProjectManager>();

			VADON_ASSERT(project_manager.get_state() == Core::ProjectManager::State::PROJECT_LOADED, "Project is in incorrect state!");
			if (m_resource_system.export_project_resources(output_path) == false)
			{
				Vadon::Core::Logger::log_error("Model system: unable to load project!\n");
				return false;
			}

			return true;
		}
	};

	ModelSystem::~ModelSystem() = default;

	Vadon::ECS::World& ModelSystem::get_ecs_world()
	{
		return m_internal->m_ecs_world;
	}

	ResourceSystem& ModelSystem::get_resource_system()
	{
		return m_internal->m_resource_system;
	}
	
	SceneSystem& ModelSystem::get_scene_system()
	{
		return m_internal->m_scene_system;
	}

	ModelSystem::ModelSystem(Core::Editor& editor)
		: System(editor)
		, m_internal(std::make_unique<Internal>(editor))
	{

	}

	bool ModelSystem::initialize()
	{
		return m_internal->initialize();
	}

	bool ModelSystem::load_project()
	{
		return m_internal->load_project(m_editor);
	}

	bool ModelSystem::export_project(std::string_view output_path)
	{
		return m_internal->export_project(m_editor, output_path);
	}
}