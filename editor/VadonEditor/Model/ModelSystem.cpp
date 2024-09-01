#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <format>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal
	{
		Vadon::ECS::World m_ecs_world;
		SceneTree m_scene_tree;

		std::vector<std::function<void()>> m_callbacks;

		Internal(Core::Editor& editor)
			: m_scene_tree(editor)
		{
		}

		bool initialize(Core::Editor& editor)
		{
			// Load resources from project root
			Core::ProjectManager& project_manager = editor.get_system<Core::ProjectManager>();
			if (project_manager.get_state() == Core::ProjectManager::State::PROJECT_ACTIVE)
			{
				if (m_scene_tree.initialize() == false)
				{
					return false;
				}

				Vadon::Core::EngineCoreInterface& engine_core = editor.get_engine_core();

				Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
				Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

				
				const Vadon::Core::FileSystem::Path root_path{ .path = "", .root = project_manager.get_active_project().root_dir_handle };
				const std::vector<std::string> scene_files = file_system.get_files_of_type(root_path, ".vdsc");
				for (const std::string& current_scene_file : scene_files)
				{
					const Vadon::Scene::ResourcePath current_scene_path{ .path = current_scene_file, .root_directory = project_manager.get_active_project().root_dir_handle };
					const Vadon::Scene::ResourceID imported_resource_id = resource_system.import_resource(current_scene_path);
					if (imported_resource_id.is_valid() == false)
					{
						Vadon::Core::Logger::log_warning(std::format("Model system: unable to load scene at \"{}\"!\n", current_scene_file));
					}
				}
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

	SceneTree& ModelSystem::get_scene_tree()
	{
		return m_internal->m_scene_tree;
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
		return m_internal->initialize(m_editor);
	}

	void ModelSystem::update()
	{
		m_internal->update();
	}
}