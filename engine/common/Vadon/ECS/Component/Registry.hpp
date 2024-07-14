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

		template<typename T>
		static void register_component_type(PoolFactoryFunction factory = nullptr)
		{
			Vadon::Utilities::TypeRegistry::register_type<T>();
			
			PoolFactoryFunction factory_impl = factory;
			if (factory_impl == nullptr)
			{
				// Use default factory
				factory_impl = +[]() { return static_cast<ComponentPoolInterface*>(new ComponentPool<T>()); };
			}

			register_component_type(get_component_type_id<T>(), factory_impl);
		}

		static ComponentPoolInterface* get_component_pool(uint32_t type_id);

		template<typename T>
		static uint32_t get_component_type_id()
		{
			return ComponentPoolInterface::get_component_type_id<T>();
		}

		VADONCOMMON_API static std::vector<uint32_t> get_component_types();
	private:
		VADONCOMMON_API static void register_component_type(uint32_t type_id, PoolFactoryFunction factory);

		struct PoolInfo
		{
			PoolFactoryFunction factory_function;
			// TODO: anything else?
		};

		std::unordered_map<uint32_t, PoolInfo> m_pool_info_lookup;
	};
}
#endif