#ifndef VADON_SCENE_RESOURCE_REGISTRY_HPP
#define VADON_SCENE_RESOURCE_REGISTRY_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>
namespace Vadon::Scene
{
	// FIXME: very similar to Component registry
	// Find some way to deduplicate?
	class ResourceRegistry
	{
	public:
		using ResourceFactoryFunction = ResourceBase*(*)();

		template<typename T>
		static void register_resource_type(ResourceFactoryFunction factory = nullptr)
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			Vadon::Utilities::TypeRegistry::register_type<T>();

			ResourceFactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<ResourceBase*>(new T()); };
			}

			register_resource_type(Vadon::Utilities::TypeRegistry::get_type_id<T>(), factory_impl);
		}

		VADONCOMMON_API static ResourceBase* create_resource(Vadon::Utilities::TypeID type_id);
	private:
		VADONCOMMON_API static void register_resource_type(Vadon::Utilities::TypeID type_id, ResourceFactoryFunction factory);

		struct ResourceTypeInfo
		{
			ResourceFactoryFunction factory_function;
			// TODO: anything else?
		};

		std::unordered_map<Vadon::Utilities::TypeID, ResourceTypeInfo> m_resource_info_lookup;
	};
}
#endif