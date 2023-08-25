#ifndef VADON_RENDER_GRAPHICSAPI_GRAPHICSMODULE_HPP
#define VADON_RENDER_GRAPHICSAPI_GRAPHICSMODULE_HPP
#include <Vadon/Core/System/EngineSystem.hpp>
#include <Vadon/Core/System/SystemModule.hpp>
#include <Vadon/Core/System/SystemModuleList.hpp>
namespace Vadon::Render
{
	class GraphicsModule : public Core::SystemModule<Core::SystemModuleList, GraphicsModule, class GraphicsAPI, class BufferSystem, class PipelineSystem,
		class RenderTargetSystem, class ShaderSystem, class TextureSystem>
	{
	};

	template<typename SysImpl>
	using GraphicsSystem = Core::EngineSystem<GraphicsModule, SysImpl>;
}
#endif