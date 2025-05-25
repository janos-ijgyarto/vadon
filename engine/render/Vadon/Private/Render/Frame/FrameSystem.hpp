#ifndef VADON_PRIVATE_RENDER_FRAME_FRAMESYSTEM_HPP
#define VADON_PRIVATE_RENDER_FRAME_FRAMESYSTEM_HPP
#include <Vadon/Render/Frame/FrameSystem.hpp>
#include <Vadon/Private/Render/Frame/Graph.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
}

namespace Vadon::Private::Render
{
	class FrameSystem final : public Vadon::Render::FrameSystem
	{
	public:
		FrameGraphHandle create_graph(const FrameGraphInfo& graph_info) override;
		bool is_graph_valid(FrameGraphHandle graph_handle) const override { return m_graph_pool.is_handle_valid(graph_handle); }
		void execute_graph(FrameGraphHandle graph_handle) override;
		void execute_graph(const FrameGraphInfo& /*graph_info*/) override {}
		void remove_graph(FrameGraphHandle graph_handle) override;
	private:
		FrameSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		using FrameGraphPool = Vadon::Utilities::ObjectPool<Vadon::Render::FrameGraph, FrameGraph>;
		FrameGraphPool m_graph_pool;

		friend class RenderSystem;
	};
}
#endif