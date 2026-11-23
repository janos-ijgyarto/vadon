#ifndef VADON_PRIVATE_RENDER_RHI_NULL_DEVICE_DEVICE_HPP
#define VADON_PRIVATE_RENDER_RHI_NULL_DEVICE_DEVICE_HPP
#include <Vadon/Render/RHI/Device/Device.hpp>
namespace Vadon::Private::Render::RHI::Null
{
	using DeviceDescription = Vadon::Render::RHI::DeviceDescription;

	using BarrierDescription = Vadon::Render::RHI::BarrierDescription;

	using BufferDescription = Vadon::Render::RHI::BufferDescription;
	using BufferHandle = Vadon::Render::RHI::BufferHandle;
	using BufferOffset = Vadon::Render::RHI::BufferOffset;

	using CommandAllocatorHandle = Vadon::Render::RHI::CommandAllocatorHandle;
	using CommandListHandle = Vadon::Render::RHI::CommandListHandle;

	using FenceDescription = Vadon::Render::RHI::FenceDescription;
	using FenceHandle = Vadon::Render::RHI::FenceHandle;

	using QueueHandle = Vadon::Render::RHI::QueueHandle;
	using QueueType = Vadon::Render::RHI::QueueType;
	using QueueSubmitDescription = Vadon::Render::RHI::QueueSubmitDescription;

	using SamplerDescription = Vadon::Render::RHI::SamplerDescription;
	using SamplerHandle = Vadon::Render::RHI::SamplerHandle;

	using GraphicsShaderDescription = Vadon::Render::RHI::GraphicsShaderDescription;
	using GraphicsShaderHandle = Vadon::Render::RHI::GraphicsShaderHandle;

	using StreamerCopyBatch = Vadon::Render::RHI::StreamerCopyBatch;
	using StreamerDescription = Vadon::Render::RHI::StreamerDescription;
	using StreamerHandle = Vadon::Render::RHI::StreamerHandle;
	using StreamBufferDataDescription = Vadon::Render::RHI::StreamBufferDataDescription;
	using StreamTextureDataDescription = Vadon::Render::RHI::StreamTextureDataDescription;

	using TextureDescription = Vadon::Render::RHI::TextureDescription;
	using TextureHandle = Vadon::Render::RHI::TextureHandle;

	using WindowHandle = Vadon::Render::RHI::WindowHandle;
	using WindowDescription = Vadon::Render::RHI::WindowDescription;

	class Device : public Vadon::Render::RHI::Device
	{
	public:
		Device(const DeviceDescription& description)
			: Vadon::Render::RHI::Device(description)
		{ }

		BufferHandle create_buffer(const BufferDescription& /*description*/) override { return BufferHandle(); }
		void remove_buffer(BufferHandle /*buffer_handle*/) override {}

		CommandAllocatorHandle create_command_allocator(QueueHandle /*queue_handle*/) override { return CommandAllocatorHandle(); }
		void reset_command_allocator(CommandAllocatorHandle /*allocator_handle*/) override {}
		void remove_command_allocator(CommandAllocatorHandle /*allocator_handle*/) override {}

		CommandListHandle create_command_list(CommandAllocatorHandle /*command_allocator*/) override { return CommandListHandle(); }
		Vadon::Render::RHI::CommandList* get_command_list(CommandListHandle /*command_list_handle*/) override { return nullptr; }
		void return_command_list(Vadon::Render::RHI::CommandList* /*command_list*/) override {}
		void remove_command_list(CommandListHandle /*command_list_handle*/) override {}

		FenceHandle create_fence(const FenceDescription& /*description*/) override { return FenceHandle(); }
		void wait_fence(FenceHandle /*fence_handle*/, uint64_t /*value*/) override {}
		uint64_t get_fence_value(FenceHandle /*fence_handle*/) override { return 0; }
		void remove_fence(FenceHandle /*fence_handle*/) override {}

		SamplerHandle create_sampler(const SamplerDescription& /*description*/) override { return SamplerHandle(); }
		void remove_sampler(SamplerHandle /*sampler_handle*/) override {}

		StreamerHandle create_streamer(const StreamerDescription& /*description*/) override { return StreamerHandle(); }
		uint32_t stream_constant_data(StreamerHandle /*streamer_handle*/, const void* /*data*/, uint32_t /*data_size*/) override { return 0; }
		StreamerCopyBatch begin_streamer_copy_batch(StreamerHandle /*streamer_handle*/) override { return 0; }
		BufferOffset stream_buffer_data(StreamerHandle /*streamer_handle*/, const StreamBufferDataDescription& /*description*/) override { return BufferOffset{}; }
		BufferOffset stream_texture_data(StreamerHandle /*streamer_handle*/, const StreamTextureDataDescription& /*description*/) override { return BufferOffset{}; }
		void end_streamer_frame(StreamerHandle /*streamer_handle*/) override {}
		void remove_streamer(StreamerHandle /*streamer_handle*/) override {}

		GraphicsShaderHandle create_graphics_shader(const GraphicsShaderDescription& /*description*/) override { return GraphicsShaderHandle(); }
		void remove_graphics_shader(GraphicsShaderHandle /*shader_handle*/) override {}

		TextureHandle create_texture(const TextureDescription& /*description*/) override { return TextureHandle(); }
		void remove_texture(TextureHandle /*texture_handle*/) override {}

		QueueHandle get_queue(QueueType /*type*/, uint32_t /*index*/) override { return QueueHandle(); }
		void queue_submit(QueueHandle /*queue_handle*/, const QueueSubmitDescription& /*queue_submit*/) override {}
		void queue_wait_idle(QueueHandle /*queue_handle*/) override {}

		WindowHandle create_window(const WindowDescription& /*description*/) override { return WindowHandle(); }
		TextureHandle get_window_next_texture(WindowHandle /*window_handle*/) override { return TextureHandle(); }
		void present_window(WindowHandle /*window_handle*/) override {}
		void remove_window(WindowHandle /*window_handle*/) override {}

		void wait_idle() override {}
	private:
	};
}
#endif