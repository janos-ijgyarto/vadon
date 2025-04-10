#include <VadonDemo/Core/Core.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Project/Project.hpp>
#include <Vadon/ECS/World/World.hpp>

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
		Vadon::Utilities::TypeRegistry::apply_property_values(&m_global_config, Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), project_info.custom_properties);

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

	void Core::override_global_config(const Vadon::Core::Project& project_info)
	{
		// Update global config values
		Vadon::Utilities::TypeRegistry::apply_property_values(&m_global_config, Vadon::Utilities::TypeRegistry::get_type_id<GlobalConfiguration>(), project_info.custom_properties);

		// Notify subsystems
		m_render.global_config_updated();
		m_model.global_config_updated();
		m_view.global_config_updated();
		m_ui.global_config_updated();
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