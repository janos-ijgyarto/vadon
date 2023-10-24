#ifndef VADON_PRIVATE_RENDER_FRAME_GRAPH_HPP
#define VADON_PRIVATE_RENDER_FRAME_GRAPH_HPP
#include <Vadon/Render/Frame/Graph.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
namespace Vadon::Private::Render
{
	using FrameGraphInfo = Vadon::Render::FrameGraphInfo;
	using FrameGraphHandle = Vadon::Render::FrameGraphHandle;

	struct FrameGraphNode
	{
		Vadon::Render::RenderPass pass;
		Vadon::Utilities::DataRange child_nodes;
	};

	struct FrameGraph
	{
		std::vector<FrameGraphNode> nodes;

		using NodeIndexVector = std::vector<size_t>;
		NodeIndexVector root_nodes;
		NodeIndexVector child_indices;

		friend class FrameSystem;
	};
}
#endif