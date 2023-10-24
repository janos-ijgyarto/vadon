#ifndef VADON_RENDER_FRAME_GRAPH_HPP
#define VADON_RENDER_FRAME_GRAPH_HPP
#include <Vadon/Render/Frame/Pass.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Render
{
	struct FrameResource
	{
		std::string name;
		// TODO: any other info?
	};

	struct FrameGraphInfo
	{
		std::string name;
		std::vector<RenderPass> passes;

		std::vector<FrameResource> targets;

		// TODO: any other info?
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(FrameGraph, FrameGraphHandle);
}
#endif