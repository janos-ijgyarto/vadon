#include <VadonDemo/Core/Core.hpp>

#include <Vadon/Core/CoreInterface.hpp>
#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Project/Project.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

namespace VadonDemo::Core
{
	Core::Core(Vadon::Core::EngineCoreInterface& engine_core)
		: m_engine_core(engine_core)
		, m_model(*this)
		, m_render(*this)
		, m_ui(*this)
		, m_view(*this)
	{
	}
	
	void Core::init_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	void Core::register_types()
	{
		GlobalConfiguration::register_type();

		Model::Model::register_types();
		Render::Render::register_types();
		UI::UI::register_types();
		View::View::register_types();
	}

	bool Core::initialize(const Vadon::Core::Project& project_info)
	{
		if (project_info.custom_data_id.is_valid() == true)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			Vadon::Scene::ResourceHandle custom_data_resource = resource_system.load_resource_base(project_info.custom_data_id);

			VADON_ASSERT(resource_system.get_resource_info(custom_data_resource).type_id == Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), "Custom data is not GlobalConfiguration!");

			m_global_config = GlobalConfigurationHandle::from_resource_handle(custom_data_resource);
		}

		if (m_render.initialize() == false)
		{
			return false;
		}

		if (m_model.initialize() == false)
		{
			return false;
		}

		if (m_view.initialize() == false)
		{
			return false;
		}

		if (m_ui.initialize() == false)
		{
			return false;
		}

		return true;
	}

	void Core::update_global_config(const GlobalConfigurationID& global_config_id)
	{
		if (global_config_id.is_valid() == true)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			Vadon::Scene::ResourceHandle custom_data_resource = resource_system.load_resource_base(global_config_id);

			VADON_ASSERT(resource_system.get_resource_info(custom_data_resource).type_id == Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), "Custom data is not GlobalConfiguration!");

			m_global_config = GlobalConfigurationHandle::from_resource_handle(custom_data_resource);
		}

		// Notify subsystems
		m_render.global_config_updated();
		m_model.global_config_updated();
		m_view.global_config_updated();
		m_ui.global_config_updated();
	}

	const GlobalConfiguration& VadonDemo::Core::Core::get_global_config() const
	{
		if(m_global_config.is_valid() == true)
		{
			Vadon::Scene::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Scene::ResourceSystem>();
			return *resource_system.get_resource(m_global_config);
		}
		else
		{
			return m_default_config;
		}
	}

	void Core::add_entity_event_callback(EntityEventCallback callback)
	{
		m_entity_callbacks.push_back(callback);
	}

	void Core::entity_added(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		dispatch_entity_event(ecs_world, EntityEvent{ .entity = entity, .type = EntityEventType::ADDED });

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		for (Vadon::ECS::EntityHandle child_entity : entity_manager.get_children(entity))
		{
			entity_added(ecs_world, child_entity);
		}
	}

	void Core::entity_removed(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		dispatch_entity_event(ecs_world, EntityEvent{ .entity = entity, .type = EntityEventType::REMOVED });

		Vadon::ECS::EntityManager& entity_manager = ecs_world.get_entity_manager();
		for (Vadon::ECS::EntityHandle child_entity : entity_manager.get_children(entity))
		{
			entity_removed(ecs_world, child_entity);
		}
	}

	void Core::dispatch_entity_event(Vadon::ECS::World& ecs_world, const EntityEvent& event)
	{
		for (const EntityEventCallback& callback : m_entity_callbacks)
		{
			callback(ecs_world, event);
		}
	}
}