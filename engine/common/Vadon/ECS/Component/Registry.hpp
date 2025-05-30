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

		template<typename T, typename Base = T>
		static void register_component_type(PoolFactoryFunction factory = nullptr)
		{
			Vadon::Utilities::TypeRegistry::register_type<T, Base>();
			
			PoolFactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<ComponentPoolInterface*>(new ComponentPool<T>()); };
			}

			register_component_type(get_component_type_id<T>(), factory_impl);
		}

		static ComponentPoolInterface* get_component_pool(Vadon::Utilities::TypeID type_id);

		template<typename T>
		static Vadon::Utilities::TypeID get_component_type_id()
		{
			return ComponentPoolInterface::get_component_type_id<T>();
		}

		VADONCOMMON_API static std::vector<Vadon::Utilities::TypeID> get_component_types();
	private:
		VADONCOMMON_API static void register_component_type(Vadon::Utilities::TypeID type_id, PoolFactoryFunction factory);

		struct PoolInfo
		{
			PoolFactoryFunction factory_function;
			// TODO: anything else?
		};

		std::unordered_map<Vadon::Utilities::TypeID, PoolInfo> m_pool_info_lookup;
	};
}
#endif