#ifndef VADON_ECS_COMPONENT_REGISTRY_HPP
#define VADON_ECS_COMPONENT_REGISTRY_HPP
#include <Vadon/ECS/Component/Pool.hpp>
namespace Vadon::ECS
{
	class ComponentPoolInterface;
	class ComponentRegistry
	{
	public:
		using PoolFactoryFunction = ComponentPoolInterface* (*)();

		// FIXME: a more elegant way to implement this?
		struct ComponentPrototypeBase
		{
			virtual ~ComponentPrototypeBase() {}

			virtual void* get_data() = 0;
		};

		template<typename T, typename Base = T>
		static void register_component_type(PoolFactoryFunction factory = nullptr)
		{
			static_assert(std::is_base_of_v<TypedComponentPool<T>, ComponentPool<T>>, "Error in Vadon::ECS: component type must be derived from Vadon::ECS::TypedComponentPool<T>!");
			Vadon::Utilities::TypeRegistry::register_type<T, Base>();
			
			PoolFactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<ComponentPoolInterface*>(new ComponentPool<T>()); };
			}

			struct ComponentPrototype : public ComponentPrototypeBase
			{
				T data;

				void* get_data() override { return &data; }
			};

			ComponentPrototype* prototype = new ComponentPrototype();
			register_component_type(ComponentPoolInterface::get_component_type_id<T>(), factory_impl, prototype);
		}

		static ComponentPoolInterface* get_component_pool(Vadon::Utilities::TypeID type_id);

		// FIXME: implement a way to do this with all registered types!
		template<typename T>
		static Vadon::Utilities::Variant get_component_property_default_value(std::string_view property_name)
		{
			return get_component_property_default_value(ComponentPoolInterface::get_component_type_id<T>(), property_name);
		}

		VADONCOMMON_API static Vadon::Utilities::Variant get_component_property_default_value(Vadon::Utilities::TypeID type_id, std::string_view property_name);

		VADONCOMMON_API static std::vector<Vadon::Utilities::TypeID> get_component_types();

		template<typename T>
		static void register_tag_type()
		{
			static_assert(std::is_empty_v<T>, "Tag must be empty type!");
			static_assert(std::is_base_of_v<TypedComponentPool<T>, TagPool<T>>, "Error in Vadon::ECS: tag pool type must be derived from Vadon::ECS::TypedComponentPool<T>!");
			Vadon::Utilities::TypeRegistry::register_type<T>();

			PoolFactoryFunction factory_impl = +[]() { return static_cast<ComponentPoolInterface*>(new TagPool<T>()); };

			// No prototype for tags, since it's not meant to have any properties!
			register_component_type(ComponentPoolInterface::get_component_type_id<T>(), factory_impl, nullptr);
		}
	private:
		VADONCOMMON_API static void register_component_type(Vadon::Utilities::TypeID type_id, PoolFactoryFunction factory, ComponentPrototypeBase* prototype);

		struct PoolInfo
		{
			PoolFactoryFunction factory_function;
			ComponentPrototypeBase* prototype = nullptr;
			// TODO: anything else?

			~PoolInfo()
			{
				if (prototype != nullptr)
				{
					delete prototype;
					prototype = nullptr;
				}
			}
		};

		std::unordered_map<Vadon::Utilities::TypeID, PoolInfo> m_pool_info_lookup;
	};
}
#endif