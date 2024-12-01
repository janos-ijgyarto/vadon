#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Scene/Resource/Registry.hpp>

#include <Vadon/Core/Environment.hpp>

namespace
{
	Vadon::Scene::ResourceRegistry& get_registry_instance()
	{
		return Vadon::Core::EngineEnvironment::get_resource_registry();
	}
}

namespace Vadon::Scene
{
	ResourceBase* ResourceRegistry::create_resource(Vadon::Utilities::TypeID type_id)
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

		assert((registry_instance.m_resource_info_lookup.find(type_id) == registry_instance.m_resource_info_lookup.end()) && "Vadon Resource registry error: resource type already registered!");

		ResourceTypeInfo resource_info;
		resource_info.factory_function = factory;

		registry_instance.m_resource_info_lookup.insert(std::make_pair(type_id, resource_info));
	}
}