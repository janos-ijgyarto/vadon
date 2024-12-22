#ifndef VADON_ECS_WORLD_WORLD_HPP
#define VADON_ECS_WORLD_WORLD_HPP
#include <Vadon/ECS/World/Event.hpp>
#include <Vadon/ECS/Entity/EntityManager.hpp>
#include <Vadon/ECS/Component/ComponentManager.hpp>
namespace Vadon::ECS
{
	class World
	{
	public:
		VADONCOMMON_API World();
		VADONCOMMON_API ~World();

		EntityManager& get_entity_manager() { return m_entity_manager; }
		ComponentManager& get_component_manager() { return m_component_manager; }

		// TODO: implement deferred removal of entities!
		VADONCOMMON_API void remove_entity(EntityHandle entity_handle);
		VADONCOMMON_API void clear();

		VADONCOMMON_API void register_event_callback(WorldEventCallback callback);

		// TODO: systems?
	private:
		void dispatch_event(const WorldEvent& event) const;

		EntityManager m_entity_manager;
		ComponentManager m_component_manager;

		std::vector<WorldEventCallback> m_event_callbacks;
	};
}
#endif