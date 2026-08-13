#include <VadonEditor/Model/ModelSystem.hpp>

#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

namespace VadonEditor::Model
{
	struct ModelSystem::Internal 
	{
		ResourceSystem m_resource_system;
		SceneSystem m_scene_system;

		Internal(Core::Application& application)
			: m_resource_system(application)
			, m_scene_system(application)
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

		void project_loaded()
		{
			m_resource_system.project_loaded();
			m_scene_system.project_loaded();
		}

		void shutdown()
		{
			m_scene_system.shutdown();
			m_resource_system.shutdown();
		}
	};

	ModelSystem::~ModelSystem() = default;

	ResourceSystem& ModelSystem::get_resource_system()
	{
		return m_internal->m_resource_system;
	}

	SceneSystem& ModelSystem::get_scene_system()
	{
		return m_internal->m_scene_system;
	}

	ModelSystem::ModelSystem(Core::Application& application)
		: m_internal(std::make_unique<Internal>(application))
	{

	}

	bool ModelSystem::initialize()
	{
		return m_internal->initialize();
	}

	void ModelSystem::project_loaded()
	{
		m_internal->project_loaded();
	}

	void ModelSystem::shutdown()
	{
		m_internal->shutdown();
	}

	void ModelSystem::simulator_initialized()
	{
		m_internal->m_resource_system.simulator_initialized();
		m_internal->m_scene_system.simulator_initialized();
	}
}