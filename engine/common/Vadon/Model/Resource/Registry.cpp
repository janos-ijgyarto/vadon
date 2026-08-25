#include <Vadon/Model/Resource/Registry.hpp>

#include <Vadon/Core/Environment.hpp>

#include <Vadon/Utilities/Debugging/Assert.hpp>

namespace
{
	Vadon::Model::ResourceRegistry& get_registry_instance()
	{
		return Vadon::Core::EngineEnvironment::get_resource_registry();
	}
}

namespace Vadon::Model
{
	Resource* ResourceRegistry::create_resource(Vadon::Utilities::TypeID type_id)
	{
		ResourceRegistry& registry_instance = get_registry_instance();

		auto resource_info_it = registry_instance.m_resource_info_lookup.find(type_id);
		if (resource_info_it == registry_instance.m_resource_info_lookup.end())
		{
			return nullptr;
		}

		return resource_info_it->second.factory_function();
	}

	void ResourceRegistry::register_resource_type(Vadon::Utilities::TypeID type_id, FactoryFunction factory)
	{
		ResourceRegistry& registry_instance = get_registry_instance();

		VADON_ASSERT((registry_instance.m_resource_info_lookup.find(type_id) == registry_instance.m_resource_info_lookup.end()), "Resource type already registered!");

		ResourceTypeInfo resource_info;
		resource_info.factory_function = factory;

		registry_instance.m_resource_info_lookup.insert(std::make_pair(type_id, resource_info));
	}
}