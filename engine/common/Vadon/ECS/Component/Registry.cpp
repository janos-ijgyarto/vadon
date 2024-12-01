#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/ECS/Component/Registry.hpp>

#include <Vadon/Core/Environment.hpp>

namespace Vadon::ECS
{
	namespace
	{
		ComponentRegistry& get_registry_instance()
		{
			return Vadon::Core::EngineEnvironment::get_component_registry();
		}
	}

	ComponentPoolInterface* ComponentRegistry::get_component_pool(Vadon::Utilities::TypeID type_id)
	{
		ComponentRegistry& registry_instance = get_registry_instance();

		auto pool_info_it = registry_instance.m_pool_info_lookup.find(type_id);
		if (pool_info_it == registry_instance.m_pool_info_lookup.end())
		{
			return nullptr;
		}

		return pool_info_it->second.factory_function();
	}

	std::vector<Vadon::Utilities::TypeID> ComponentRegistry::get_component_types()
	{
		const ComponentRegistry& registry_instance = get_registry_instance();
		std::vector<Vadon::Utilities::TypeID> type_id_list;

		type_id_list.reserve(registry_instance.m_pool_info_lookup.size());

		for (auto pool_info_it : registry_instance.m_pool_info_lookup)
		{
			type_id_list.push_back(pool_info_it.first);
		}

		return type_id_list;
	}

	void ComponentRegistry::register_component_type(Vadon::Utilities::TypeID type_id, PoolFactoryFunction factory)
	{
		ComponentRegistry& registry_instance = get_registry_instance();

		assert((registry_instance.m_pool_info_lookup.find(type_id) == registry_instance.m_pool_info_lookup.end()) && "Vadon ECS error: component type already registered!");

		PoolInfo pool_info;
		pool_info.factory_function = factory;

		registry_instance.m_pool_info_lookup.insert(std::make_pair(type_id, pool_info));
	}
}