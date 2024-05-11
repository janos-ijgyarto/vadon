#ifndef VADON_RENDER_RENDERMODULE_HPP
#define VADON_RENDER_RENDERMODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Render::Canvas
{
	class CanvasSystem;
}
namespace Vadon::Render
{
	class RenderModule : public Core::SystemModule<Core::SystemModuleList, RenderModule, class Canvas::CanvasSystem, class FrameSystem>
	{
	};

	template<typename SysImpl>
	using RenderSystem = Core::System<RenderModule, SysImpl>;
}
#endif