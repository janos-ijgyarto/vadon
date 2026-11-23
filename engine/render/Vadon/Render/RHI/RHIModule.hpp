#ifndef VADON_RENDER_RHI_RHIMODULE_HPP
#define VADON_RENDER_RHI_RHIMODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Render::RHI
{
	class RHIModule : public Core::SystemModule<Core::SystemModuleList, RHIModule, class RHISystem>
	{
	};

	template<typename SysImpl>
	using RHISystemBase = Core::System<RHIModule, SysImpl>;
}
#endif