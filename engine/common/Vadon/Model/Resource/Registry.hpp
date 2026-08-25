#ifndef VADON_MODEL_RESOURCE_REGISTRY_HPP
#define VADON_MODEL_RESOURCE_REGISTRY_HPP
#include <Vadon/Model/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace Vadon::Model
{
	class ResourceSystem;

	// FIXME: very similar to Component registry
	// Find some way to deduplicate?
	class ResourceRegistry
	{
	public:
		using FactoryFunction = Resource*(*)();

		template<typename T, typename Base = T>
		static void register_resource_type(FactoryFunction factory = nullptr)
		{
			static_assert(std::is_base_of_v<Resource, T>);
			Vadon::Utilities::TypeRegistry::register_type<T, Base>();

			FactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<Resource*>(new T()); };
			}

			register_resource_type(Vadon::Utilities::TypeRegistry::get_type_id<T>(), factory_impl);
		}

		VADONCOMMON_API static Resource* create_resource(Vadon::Utilities::TypeID type_id);
	private:
		VADONCOMMON_API static void register_resource_type(Vadon::Utilities::TypeID type_id, FactoryFunction factory);

		struct ResourceTypeInfo
		{
			FactoryFunction factory_function;
			// TODO: anything else?
		};

		std::unordered_map<Vadon::Utilities::TypeID, ResourceTypeInfo> m_resource_info_lookup;
	};
}
#endif