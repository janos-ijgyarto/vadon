#ifndef VADON_SCENE_RESOURCE_REGISTRY_HPP
#define VADON_SCENE_RESOURCE_REGISTRY_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace Vadon::Scene
{
	class ResourceSystemInterface;

	// FIXME: very similar to Component registry
	// Find some way to deduplicate?
	class ResourceRegistry
	{
	public:
		using FactoryFunction = ResourceBase*(*)();
		using SerializerFunction = bool(*)(ResourceSystemInterface&, Vadon::Utilities::Serializer&, ResourceBase&);

		template<typename T, typename Base = T>
		static void register_resource_type(FactoryFunction factory = nullptr)
		{
			static_assert(std::is_base_of_v<ResourceBase, T>);
			Vadon::Utilities::TypeRegistry::register_type<T, Base>();

			FactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<ResourceBase*>(new T()); };
			}

			register_resource_type(Vadon::Utilities::TypeRegistry::get_type_id<T>(), factory_impl);
		}

		template<typename T>
		static void register_serializer(SerializerFunction serializer)
		{
			register_serializer(Vadon::Utilities::TypeRegistry::get_type_id<T>(), serializer);
		}

		template<typename T>
		static SerializerFunction get_serializer()
		{
			get_serializer(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		VADONCOMMON_API static ResourceBase* create_resource(Vadon::Utilities::TypeID type_id);
		VADONCOMMON_API static SerializerFunction get_serializer(Vadon::Utilities::TypeID type_id);
	private:
		VADONCOMMON_API static void register_resource_type(Vadon::Utilities::TypeID type_id, FactoryFunction factory);
		VADONCOMMON_API static void register_serializer(Vadon::Utilities::TypeID type_id, SerializerFunction serializer);

		struct ResourceTypeInfo
		{
			FactoryFunction factory_function;
			SerializerFunction serializer = nullptr;
			// TODO: anything else?
		};

		std::unordered_map<Vadon::Utilities::TypeID, ResourceTypeInfo> m_resource_info_lookup;
	};
}
#endif