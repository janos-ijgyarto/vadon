#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Core/Component.hpp>

#include <Vadon/Core/Environment.hpp>
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

	bool Core::initialize()
	{
		CoreComponent::register_component();

		Model::Model::register_types();
		Render::Render::register_types();
		UI::UI::register_types();
		View::View::register_types();

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