#ifndef VADON_SCENE_RESOURCE_REGISTRY_HPP
#define VADON_SCENE_RESOURCE_REGISTRY_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace Vadon::Scene
{
	class ResourceSystem;

	// FIXME: very similar to Component registry
	// Find some way to deduplicate?
	class ResourceRegistry
	{
	public:
		using FactoryFunction = Resource*(*)();

		// We pass the resource system instance that called this
		// FIXME: implement a way to retrieve "context" from the engine
		// That way, if needed, endpoint can access engine from resource system, and from there access the relevant context
		using SerializerFunction = bool(*)(ResourceSystem&, Vadon::Utilities::Serializer&, Resource&);

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

		template<typename T>
		static void register_resource_serializer(SerializerFunction serializer)
		{
			register_resource_serializer(Vadon::Utilities::TypeRegistry::get_type_id<T>(), serializer);
		}

		template<typename T>
		static SerializerFunction get_resource_serializer()
		{
			return get_resource_serializer(Vadon::Utilities::TypeRegistry::get_type_id<T>());
		}

		VADONCOMMON_API static Resource* create_resource(Vadon::Utilities::TypeID type_id);
		VADONCOMMON_API static SerializerFunction get_resource_serializer(Vadon::Utilities::TypeID type_id);
	private:
		VADONCOMMON_API static void register_resource_type(Vadon::Utilities::TypeID type_id, FactoryFunction factory);
		VADONCOMMON_API static void register_resource_serializer(Vadon::Utilities::TypeID type_id, SerializerFunction serializer);

		struct SerializerInfo
		{
			SerializerFunction function = nullptr;
			void* context = nullptr;

			bool is_valid() const { return function != nullptr; }
		};

		struct ResourceTypeInfo
		{
			FactoryFunction factory_function;
			SerializerInfo serializer;
			// TODO: anything else?
		};

		std::unordered_map<Vadon::Utilities::TypeID, ResourceTypeInfo> m_resource_info_lookup;
	};
}
#endif