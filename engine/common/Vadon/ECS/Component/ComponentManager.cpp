#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/ComponentManager.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

namespace Vadon::ECS
{	
	ComponentManager::~ComponentManager()
	{
		for (auto& current_pool_pair : m_component_pools)
		{
			delete current_pool_pair.second;
		}
	}

	void* ComponentManager::add_component(EntityHandle entity, ComponentID type_id)
	{
		ComponentPoolInterface* pool = get_component_pool(type_id);
		if (pool != nullptr)
		{
			void* new_component_ptr = pool->add_component(entity);
			if (new_component_ptr != nullptr)
			{
				dispatch_component_event(*pool, ComponentEvent{ .owner = entity, .type_id = type_id, .event_type = ComponentEventType::ADDED });
			}
			return new_component_ptr;
		}

		return nullptr;
	}

	bool ComponentManager::has_component(EntityHandle entity, ComponentID type_id) const
	{
		const ComponentPoolInterface* pool = find_component_pool(type_id);
		if (pool != nullptr)
		{
			return pool->get_component(entity) != nullptr;
		}

		return false;
	}

	void* ComponentManager::get_component(EntityHandle entity, ComponentID type_id)
	{
		ComponentPoolInterface* pool = find_component_pool(type_id);
		if (pool != nullptr)
		{
			return pool->get_component(entity);
		}

		return nullptr;
	}

	void ComponentManager::remove_component(EntityHandle entity, ComponentID type_id)
	{
		ComponentPoolInterface* pool = find_component_pool(type_id);
		if (pool != nullptr)
		{
			dispatch_component_event(*pool, ComponentEvent{ .owner = entity, .type_id = type_id, .event_type = ComponentEventType::REMOVED });
			pool->remove_component(entity);
		}
	}

	ComponentIDList ComponentManager::get_component_list(EntityHandle entity) const
	{
		ComponentIDList component_type_ids;
		for (auto& current_pool_pair : m_component_pools)
		{
			if (current_pool_pair.second->has_component(entity))
			{
				component_type_ids.push_back(current_pool_pair.first);
			}
		}

		return component_type_ids;
	}

	void ComponentManager::register_event_callback(ComponentEventCallback callback, ComponentID type_id)
	{
		ComponentPoolInterface* component_pool = get_component_pool(type_id);
		component_pool->m_event_callbacks.push_back(callback);
	}

	ComponentManager::ComponentManager() = default;

	void ComponentManager::remove_entity(EntityHandle entity)
	{
		for (auto& current_pool_pair : m_component_pools)
		{
			current_pool_pair.second->remove_component(entity);
		}
	}

	void ComponentManager::remove_entity_batch(const EntityList& entity_batch)
	{
		ComponentEvent remove_event;
		remove_event.event_type = ComponentEventType::REMOVED;
		for (auto& current_pool_pair : m_component_pools)
		{
			remove_event.type_id = current_pool_pair.first;
			ComponentPoolInterface* component_pool = current_pool_pair.second;
			for (const EntityHandle& current_entity : entity_batch)
			{
				remove_event.owner = current_entity;
				dispatch_component_event(*component_pool, remove_event);
				component_pool->remove_component(current_entity);
			}
		}
	}

	void ComponentManager::dispatch_component_event(const ComponentPoolInterface& pool, const ComponentEvent& event)
	{
		for (auto& current_callback : pool.m_event_callbacks)
		{
			current_callback(event);
		}
	}

	const ComponentPoolInterface* ComponentManager::find_component_pool(ComponentID type_id) const
	{
		auto pool_it = m_component_pools.find(type_id);
		if (pool_it == m_component_pools.end())
		{
			return nullptr;
		}

		return pool_it->second;
	}

	ComponentPoolInterface* ComponentManager::get_component_pool(ComponentID type_id)
	{
		ComponentPoolInterface* component_pool = find_component_pool(type_id);
		if (component_pool == nullptr)
		{
			component_pool = ComponentRegistry::get_component_pool(type_id);
			assert(component_pool && "Component Manager error: component type not registered!");
			m_component_pools[type_id] = component_pool;
		}

		return component_pool;
	}

	void ComponentManager::internal_get_component_tuple(EntityHandle entity, ComponentIDSpan component_ids, ComponentSpan components)
	{
		auto component_ptr_it = components.begin();
		for (ComponentID current_component_id : component_ids)
		{
			*component_ptr_it = get_component(entity, current_component_id);
			++component_ptr_it;
		}
	}
}