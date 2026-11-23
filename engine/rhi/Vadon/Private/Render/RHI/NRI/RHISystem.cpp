#include <Vadon/Private/Render/RHI/NRI/RHISystem.hpp>

namespace Vadon::Private::Render::RHI::NRI
{
	RHISystem::RHISystem(Vadon::Core::EngineCoreInterface& core, const DeviceDescription& description)
		: RHISystemBase(core)
		, m_device(description)
	{

	}

	bool RHISystem::initialize()
	{
		return m_device.initialize();
	}

	void RHISystem::shutdown()
	{
		m_device.shutdown();
	}
}