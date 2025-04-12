#ifndef VADONDEMO_CORE_CORE_HPP
#define VADONDEMO_CORE_CORE_HPP
#include <VadonDemo/Core/Configuration.hpp>
#include <VadonDemo/Model/Model.hpp>
#include <VadonDemo/Render/Render.hpp>
#include <VadonDemo/UI/UI.hpp>
#include <VadonDemo/View/View.hpp>

#include <functional>
namespace Vadon::Core
{
	class EngineEnvironment;
	class EngineCoreInterface;
	struct Project;
}
namespace VadonDemo::Core
{
	enum class EntityEventType
	{
		ADDED,
		REMOVED
	};

	struct EntityEvent
	{
		Vadon::ECS::EntityHandle entity;
		EntityEventType type;
	};

	class Core
	{
	public:
		using EntityEventCallback = std::function<void(Vadon::ECS::World&, const EntityEvent&)>;

		VADONDEMO_API Core(Vadon::Core::EngineCoreInterface& engine_core);		
		VADONDEMO_API static void init_environment(Vadon::Core::EngineEnvironment& environment);
		VADONDEMO_API static void register_types();

		VADONDEMO_API bool initialize(const Vadon::Core::Project& project_info);
		VADONDEMO_API void override_global_config(const Vadon::Core::Project& project_info);
		const GlobalConfiguration& get_global_config() const { return m_global_config; }

		Vadon::Core::EngineCoreInterface& get_engine_core() { return m_engine_core; }

		Model::Model& get_model() { return m_model; }
		Render::Render& get_render() { return m_render; }
		UI::UI& get_ui() { return m_ui; }
		View::View& get_view() { return m_view; }

		VADONDEMO_API void add_entity_event_callback(EntityEventCallback callback);

		VADONDEMO_API void entity_added(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
		VADONDEMO_API void entity_removed(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity);
	private:
		void dispatch_entity_event(Vadon::ECS::World& ecs_world, const EntityEvent& event);

		Vadon::Core::EngineCoreInterface& m_engine_core;

		Model::Model m_model;
		Render::Render m_render;
		UI::UI m_ui;
		View::View m_view;

		std::vector<EntityEventCallback> m_entity_callbacks;

		GlobalConfiguration m_global_config;
	};
}
#endif