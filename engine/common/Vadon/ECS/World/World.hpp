#ifndef VADON_ECS_WORLD_WORLD_HPP
#define VADON_ECS_WORLD_WORLD_HPP
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

		VADONCOMMON_API void remove_pending_entities();
		VADONCOMMON_API void clear();

		// TODO: systems?
	private:
		EntityManager m_entity_manager;
		ComponentManager m_component_manager;
	};
}
#endif