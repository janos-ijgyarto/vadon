#ifndef VADON_RENDER_GRAPHICSAPI_GRAPHICSMODULE_HPP
#define VADON_RENDER_GRAPHICSAPI_GRAPHICSMODULE_HPP
#include <Vadon/Core/System/System.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
namespace Vadon::Render
{
	class GraphicsModule : public Core::SystemModule<Core::SystemModuleList, GraphicsModule, class GraphicsAPI, class BufferSystem, class PipelineSystem,
		class RenderTargetSystem, class ShaderSystem, class TextureSystem>
	{
	};

	template<typename SysImpl>
	using GraphicsSystem = Core::System<GraphicsModule, SysImpl>;
}
#endif