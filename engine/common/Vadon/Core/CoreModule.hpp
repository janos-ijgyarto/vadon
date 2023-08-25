#ifndef VADON_CORE_COREMODULE_HPP
#define VADON_CORE_COREMODULE_HPP
#include <Vadon/Core/System/EngineSystem.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
#include <Vadon/Core/System/SystemModuleList.hpp>
namespace Vadon::Core
{
	class CoreModule : public SystemModule<SystemModuleList, CoreModule, class TaskSystem>
	{
	};

	template<typename SysImpl>
	using CoreSystem = EngineSystem<CoreModule, SysImpl>;
}
#endif