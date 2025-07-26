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

	Vadon::Utilities::Variant ComponentRegistry::get_component_property_default_value(Vadon::Utilities::TypeID type_id, std::string_view property_name)
	{
		const ComponentRegistry& registry_instance = get_registry_instance();
		auto pool_info_it = registry_instance.m_pool_info_lookup.find(type_id);
		if (pool_info_it == registry_instance.m_pool_info_lookup.end())
		{
			return Vadon::Utilities::Variant();
		}

		return Vadon::Utilities::TypeRegistry::get_property(pool_info_it->second.prototype->get_data(), type_id, property_name);
	}

	std::vector<Vadon::Utilities::TypeID> ComponentRegistry::get_component_types()
	{
		const ComponentRegistry& registry_instance = get_registry_instance();
		std::vector<Vadon::Utilities::TypeID> type_id_list;

		type_id_list.reserve(registry_instance.m_pool_info_lookup.size());

		for (const auto& pool_info_it : registry_instance.m_pool_info_lookup)
		{
			type_id_list.push_back(pool_info_it.first);
		}

		return type_id_list;
	}

	void ComponentRegistry::register_component_type(Vadon::Utilities::TypeID type_id, PoolFactoryFunction factory, ComponentPrototypeBase* prototype)
	{
		ComponentRegistry& registry_instance = get_registry_instance();

		VADON_ASSERT((registry_instance.m_pool_info_lookup.find(type_id) == registry_instance.m_pool_info_lookup.end()), "Component type already registered!");

		PoolInfo pool_info;
		pool_info.factory_function = factory;

		auto new_entry = registry_instance.m_pool_info_lookup.insert(std::make_pair(type_id, pool_info));

		// Set the prototype in the info object
		new_entry.first->second.prototype = prototype;
	}
}