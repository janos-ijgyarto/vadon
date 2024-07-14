#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/Scene/SceneTree.hpp>

#include <Vadon/ECS/World/World.hpp>

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

		bool initialize()
		{
			if (m_scene_tree.initialize() == false)
			{
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
		return m_internal->initialize();
	}

	void ModelSystem::update()
	{
		m_internal->update();
	}
}