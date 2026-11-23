#include <Vadon/Private/Render/RHI/RHISystem.hpp>

#include <Vadon/Private/Render/RHI/Null/RHISystem.hpp>

#if defined(VADON_RHI_DEFAULT)
	#if defined(VADON_PLATFORM_WIN32)
		#if !defined(VADON_RHI_NRI)
			#define VADON_RHI_NRI
		#endif
	#endif
	// TODO: other platforms!
#endif

#ifdef VADON_RHI_NRI
#include <Vadon/Private/Render/RHI/NRI/RHISystem.hpp>
#endif

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Configuration.hpp>

namespace Vadon::Private::Render::RHI
{
	void RHISystemBase::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	RHISystemBase::Implementation RHISystemBase::get_rhi(Vadon::Core::EngineCoreInterface& core)
	{
#ifdef VADON_RHI_NRI
		if (Vadon::Utilities::to_bool(core.get_config().render_config.flags & Vadon::Core::RenderConfigurationFlags::DISABLE_RENDERING) == true)
		{
			return get_null_rhi(core);
		}

		// TODO: make these options configurable
		Vadon::Render::RHI::DeviceDescription device_description;
		device_description.graphics_api = Vadon::Render::RHI::GraphicsAPI::D3D12;

		return std::make_unique<NRI::RHISystem>(core, device_description);
#else
		return get_null_rhi(core);
#endif
	}

	RHISystemBase::Implementation RHISystemBase::get_null_rhi(Vadon::Core::EngineCoreInterface& core)
	{
		return std::make_unique<Null::RHISystem>(core);
	}
}