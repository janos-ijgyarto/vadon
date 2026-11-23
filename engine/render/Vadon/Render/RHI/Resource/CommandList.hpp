#ifndef VADON_RENDER_RHI_RESOURCE_COMMANDLIST_HPP
#define VADON_RENDER_RHI_RESOURCE_COMMANDLIST_HPP
#include <Vadon/Math/Color.hpp>
#include <Vadon/Math/Rectangle.hpp>

#include <Vadon/Render/RHI/Device/Barrier.hpp>
#include <Vadon/Render/RHI/Device/Streamer.hpp>
#include <Vadon/Render/RHI/Resource/Texture.hpp>
namespace Vadon::Render::RHI
{
	VADON_DECLARE_TYPED_POOL_HANDLE(CommandAllocator, CommandAllocatorHandle);

	enum class RenderPassLoadOperation
	{
		LOAD,
		CLEAR
	};

	enum class RenderPassStoreOperation
	{
		STORE,
		DISCARD,
		NONE
	};

	struct RenderPassClearValue
	{
		Vadon::Math::ColorRGBA color;
		float depth;
		uint8_t stencil;
	};

	struct RenderPassTarget
	{
		TextureHandle texture;
		RenderPassClearValue clear_value;
		RenderPassLoadOperation load_op;
		RenderPassStoreOperation store_op;
		// TODO: multisample resolve
	};

	struct RenderPassDescription
	{
		std::span<RenderPassTarget> targets;
		RenderPassTarget depth;
		RenderPassTarget stencil;
	};

	struct Viewport
	{
		Vadon::Math::Rectangle rectangle;
		Vadon::Math::Vector2 depth_min_max;
	};

	struct DrawDescription
	{
		uint32_t vertex_count;
		uint32_t instance_count;
		uint32_t base_vertex;
		uint32_t base_instance;
	};

	struct DrawIndexedDescription
	{
		uint32_t index_count;
		uint32_t instance_count;
		uint32_t base_index;
		int32_t base_vertex;
		uint32_t base_instance;
	};

	class CommandList
	{
	public:
		// NOTE: this is for the command list itself (reuse, etc.)
		virtual void begin() = 0;
		virtual void end() = 0;

		// NOTE: this is built around the "render pass" paradigm, but essentially we just set RTs and DS and do all draws in between
		// Have to alternate between this and compute dispatches where relevant
		virtual void begin_rendering(const RenderPassDescription& pass_description) = 0;
		virtual void end_rendering() = 0;

		virtual void set_viewports(std::span<Viewport> viewports) = 0;
		virtual void set_scissors(std::span<Vadon::Math::RectangleInt> scissor_rectangles) = 0;

		virtual void barrier(const BarrierDescription& description) = 0;

		virtual void draw(const DrawDescription& description) = 0;
		virtual void draw_indexed(const DrawIndexedDescription& description) = 0;

		virtual void copy_streamed_data(StreamerHandle streamer_handle, StreamerCopyBatch copy_batch) = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(CommandList, CommandListHandle);
}
#endif