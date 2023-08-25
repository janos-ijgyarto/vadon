#ifndef VADON_RENDER_FRAME_GRAPH_HPP
#define VADON_RENDER_FRAME_GRAPH_HPP
#include <Vadon/Render/Frame/Pass.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Render
{
	struct FrameRenderTarget
	{
		std::string name;
		RenderTargetHandle target;
	};

	struct FrameGraphInfo
	{
		std::string name;
		std::vector<RenderPass> passes;

		std::vector<FrameRenderTarget> targets;

		// TODO: any other info?
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(FrameGraph, FrameGraphHandle);
}
#endif