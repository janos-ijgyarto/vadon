#include <Vadon/Private/Render/RHI/Null/RHISystem.hpp>

namespace Vadon::Private::Render::RHI::Null
{
	RHISystem::RHISystem(Vadon::Core::EngineCoreInterface& core)
		: RHISystemBase(core)
		, m_device(Vadon::Render::RHI::DeviceDescription{})
	{
	}

	bool RHISystem::initialize()
	{
		return true;
	}

	void RHISystem::shutdown()
	{

	}
}