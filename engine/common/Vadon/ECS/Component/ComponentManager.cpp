#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/ComponentManager.hpp>

#include <Vadon/ECS/Component/Registry.hpp>

namespace Vadon::ECS
{	
	const void* ComponentHandle::get_raw() const
	{
		return m_pool != nullptr ? m_pool->get_component(m_owner) : nullptr;
	}

	bool ComponentHandle::is_valid() const
	{
		return m_pool != nullptr ? m_pool->has_component(m_owner) : false;
	}

	ComponentManager::~ComponentManager()
	{
		for (auto& current_pool_pair : m_component_pools)
		{
			delete current_pool_pair.second;
		}
	}

	ComponentHandle ComponentManager::add_component(EntityHandle entity, ComponentID type_id)
	{
		ComponentPoolInterface* pool = get_component_pool(type_id);
		if (pool != nullptr)
		{
			void* new_component_ptr = pool->add_component(entity);
			VADON_ASSERT(new_component_ptr != nullptr, "Failed to create component!");
			// TODO: check return value?
			return ComponentHandle(pool, entity);
		}

		return ComponentHandle(nullptr, entity);
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

	ComponentHandle ComponentManager::get_component(EntityHandle entity, ComponentID type_id) const
	{
		ComponentPoolInterface* pool = find_component_pool(type_id);
		return ComponentHandle(pool, entity);
	}

	void ComponentManager::remove_component(EntityHandle entity, ComponentID type_id)
	{
		ComponentPoolInterface* pool = find_component_pool(type_id);
		if (pool != nullptr)
		{
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
		for (auto& current_pool_pair : m_component_pools)
		{
			ComponentPoolInterface* component_pool = current_pool_pair.second;
			for (const EntityHandle& current_entity : entity_batch)
			{
				component_pool->remove_component(current_entity);
			}
		}
	}

	ComponentPoolInterface* ComponentManager::find_component_pool(ComponentID type_id) const
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
			VADON_ASSERT(component_pool != nullptr, "Component type not registered!");
			m_component_pools[type_id] = component_pool;
		}

		return component_pool;
	}

	void ComponentManager::clear()
	{
		for (auto& current_pool_pair : m_component_pools)
		{
			current_pool_pair.second->clear();
		}
	}
}