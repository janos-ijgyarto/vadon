#ifndef VADON_CORE_SYSTEM_SYSTEM_HPP
#define VADON_CORE_SYSTEM_SYSTEM_HPP
#include <Vadon/Core/System/SystemRegistry.hpp>
namespace Vadon::Core
{
	class SystemBase
	{
	protected:
		SystemBase(SystemRegistry& registry, size_t module_index, size_t system_index)			
		{
			registry.register_system(this, module_index, system_index);
		}		
	};

	template<typename Module, typename SysImpl>
	class System : public SystemBase
	{
	public:
		using _Module = Module;
		using _System = SysImpl;

		static constexpr size_t get_type_id() { return _Module::template get_system_id<_System>(); }
	protected:
		System(SystemRegistry& registry)
			: SystemBase(registry, _Module::get_type_id(), _Module::template get_system_id<_System>())
		{
		}
	};
}
#endif