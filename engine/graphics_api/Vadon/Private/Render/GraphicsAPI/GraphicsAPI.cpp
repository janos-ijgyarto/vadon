#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Null/GraphicsAPI.hpp>

#ifdef VADON_GRAPHICS_API_DIRECTX
#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#endif

#include <Vadon/Core/Environment.hpp>

namespace Vadon::Private::Render
{
	void GraphicsAPIBase::init_engine_environment(Vadon::Core::EngineEnvironment& environment)
	{
		Vadon::Core::EngineEnvironment::initialize(environment);
	}

	GraphicsAPIBase::Implementation GraphicsAPIBase::get_graphics_api(Vadon::Core::EngineCoreInterface& core)
	{
#ifdef VADON_GRAPHICS_API_DIRECTX
		return std::make_unique<DirectX::GraphicsAPI>(core);
#else
		return get_null_graphics_api(core);
#endif
	}

	GraphicsAPIBase::Implementation GraphicsAPIBase::get_null_graphics_api(Vadon::Core::EngineCoreInterface& core)
	{
		return std::make_unique<Null::GraphicsAPI>(core);
	}
}