#ifndef VADON_RENDER_FRAME_FRAMESYSTEM_HPP
#define VADON_RENDER_FRAME_FRAMESYSTEM_HPP
#include <Vadon/Render/RenderModule.hpp>
#include <Vadon/Render/Frame/Graph.hpp>
namespace Vadon::Render
{
	class FrameSystem : public RenderSystem<FrameSystem>
	{
	public:
		virtual FrameGraphHandle create_graph(const FrameGraphInfo& graph_info) = 0;
		virtual bool is_graph_valid(FrameGraphHandle graph_handle) const = 0;
		virtual void execute_graph(FrameGraphHandle graph_handle) = 0;
		virtual void execute_graph(const FrameGraphInfo& graph_info) = 0; // Dynamic graph (processing result is not saved)
		virtual void remove_graph(FrameGraphHandle graph_handle) = 0;
	protected:
		FrameSystem(Core::EngineCoreInterface& core) 
			: System(core)
		{}
	};
}
#endif  