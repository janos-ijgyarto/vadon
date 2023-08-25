#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>

#include <Vadon/Private/Render/GraphicsAPI/Dummy/GraphicsAPI.hpp>

#ifdef VADON_GRAPHICS_API_DIRECTX
#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>
#endif

namespace Vadon::Private::Render
{
	GraphicsAPIBase::Implementation GraphicsAPIBase::get_graphics_api(Vadon::Core::EngineCoreInterface& core)
	{
#ifdef VADON_GRAPHICS_API_DIRECTX
		return std::make_unique<DirectX::GraphicsAPI>(core);
#else
		return get_dummy_graphics_api(core);
#endif
	}

	GraphicsAPIBase::Implementation GraphicsAPIBase::get_dummy_graphics_api(Vadon::Core::EngineCoreInterface& core)
	{
		return std::make_unique<Dummy::GraphicsAPI>(core);
	}
}