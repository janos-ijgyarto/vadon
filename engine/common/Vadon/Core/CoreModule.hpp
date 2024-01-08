#ifndef VADON_CORE_COREMODULE_HPP
#define VADON_CORE_COREMODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Core
{
	class CoreModule : public SystemModule<SystemModuleList, CoreModule, class TaskSystem>
	{
	};

	template<typename SysImpl>
	using CoreSystem = System<CoreModule, SysImpl>;
}
#endif