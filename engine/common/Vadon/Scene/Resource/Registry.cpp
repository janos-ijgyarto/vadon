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

	ResourceRegistry::SerializerFunction ResourceRegistry::get_resource_serializer(Vadon::Utilities::TypeID type_id)
	{
		ResourceRegistry& registry_instance = get_registry_instance();

		auto resource_info_it = registry_instance.m_resource_info_lookup.find(type_id);
		if (resource_info_it == registry_instance.m_resource_info_lookup.end())
		{
			return nullptr;
		}

		return resource_info_it->second.serializer.function;
	}

	void ResourceRegistry::register_resource_type(Vadon::Utilities::TypeID type_id, FactoryFunction factory)
	{
		ResourceRegistry& registry_instance = get_registry_instance();

		VADON_ASSERT((registry_instance.m_resource_info_lookup.find(type_id) == registry_instance.m_resource_info_lookup.end()), "Resource type already registered!");

		ResourceTypeInfo resource_info;
		resource_info.factory_function = factory;

		registry_instance.m_resource_info_lookup.insert(std::make_pair(type_id, resource_info));
	}

	void ResourceRegistry::register_resource_serializer(Vadon::Utilities::TypeID type_id, SerializerFunction serializer)
	{
		ResourceRegistry& registry_instance = get_registry_instance();

		auto resource_info_it = registry_instance.m_resource_info_lookup.find(type_id);
		VADON_ASSERT((resource_info_it != registry_instance.m_resource_info_lookup.end()), "Resource type not registered, cannot register serializer!");

		ResourceTypeInfo& resource_info = resource_info_it->second;
		resource_info.serializer.function = serializer;
	}
}