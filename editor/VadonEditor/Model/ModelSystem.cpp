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

		std::vector<std::function<void()>> m_callbacks;

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

			if (project_manager.get_state() != Core::ProjectManager::State::PROJECT_LOADED)
			{
				editor.get_engine_core().log_error("Model system: unable to load project!\n");
				return false;
			}

			if (m_resource_system.load_project_resources() == false)
			{
				editor.get_engine_core().log_error("Model system: unable to load project!\n");
				return false;
			}

			return true;
		}

		void update()
		{
			// Run callbacks
			for (auto& current_callback : m_callbacks)
			{
				current_callback();
			}
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

	void ModelSystem::add_callback(std::function<void()> callback)
	{
		m_internal->m_callbacks.push_back(callback);
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

	void ModelSystem::update()
	{
		m_internal->update();
	}
}