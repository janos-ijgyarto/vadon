#ifndef VADON_RENDER_RHI_RHISYSTEM_HPP
#define VADON_RENDER_RHI_RHISYSTEM_HPP
#include <Vadon/Render/RHI/RHIModule.hpp>
namespace Vadon::Render::RHI
{
	class Device;

	class RHISystem : public RHISystemBase<RHISystem>
	{
	public:
		virtual Device& get_device() = 0;
	protected:
		RHISystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif