#ifndef VADON_PRIVATE_RENDER_RHI_NRI_DEVICE_DEVICE_HPP
#define VADON_PRIVATE_RENDER_RHI_NRI_DEVICE_DEVICE_HPP
#include <Vadon/Render/RHI/Device/Device.hpp>
#include <nri/NRIDescs.h>
#include <memory>
namespace nri
{
	struct CoreInterface;
	struct Streamer;
	struct StreamerInterface;
}
namespace Vadon::Private::Render::RHI::NRI
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
		Device(const DeviceDescription& description);
		~Device();

		BufferHandle create_buffer(const BufferDescription& description) override;
		void remove_buffer(BufferHandle buffer_handle) override;

		CommandAllocatorHandle create_command_allocator(QueueHandle queue_handle) override;
		void reset_command_allocator(CommandAllocatorHandle allocator_handle) override;
		void remove_command_allocator(CommandAllocatorHandle allocator_handle) override;

		CommandListHandle create_command_list(CommandAllocatorHandle command_allocator) override;
		Vadon::Render::RHI::CommandList* get_command_list(CommandListHandle command_list_handle) override;
		void return_command_list(Vadon::Render::RHI::CommandList* command_list) override;
		void remove_command_list(CommandListHandle command_list_handle) override;

		FenceHandle create_fence(const FenceDescription& description) override;
		void wait_fence(FenceHandle fence_handle, uint64_t value) override;
		uint64_t get_fence_value(FenceHandle fence_handle) override;
		void remove_fence(FenceHandle fence_handle) override;

		SamplerHandle create_sampler(const SamplerDescription& description) override;
		void remove_sampler(SamplerHandle sampler_handle) override;

		StreamerHandle create_streamer(const StreamerDescription& description) override;
		uint32_t stream_constant_data(StreamerHandle streamer_handle, const void* data, uint32_t data_size) override;
		StreamerCopyBatch begin_streamer_copy_batch(StreamerHandle streamer_handle) override;
		BufferOffset stream_buffer_data(StreamerHandle streamer_handle, const StreamBufferDataDescription& description) override;
		BufferOffset stream_texture_data(StreamerHandle streamer_handle, const StreamTextureDataDescription& description) override;
		void end_streamer_frame(StreamerHandle streamer_handle) override;
		void remove_streamer(StreamerHandle streamer_handle) override;

		nri::Streamer& get_nri_streamer(StreamerHandle streamer_handle);

		nri::Buffer* get_nri_buffer(BufferHandle buffer_handle);
		nri::Texture* get_nri_texture(TextureHandle texture_handle);
		nri::Descriptor* get_nri_texture_rtv(TextureHandle texture_handle);

		static nri::StageBits get_nri_stage_bits(Vadon::Render::RHI::ShaderStage stages);

		GraphicsShaderHandle create_graphics_shader(const GraphicsShaderDescription& description) override;
		void remove_graphics_shader(GraphicsShaderHandle shader_handle) override;

		TextureHandle create_texture(const TextureDescription& description) override;
		void remove_texture(TextureHandle texture_handle) override;

		QueueHandle get_queue(QueueType type, uint32_t index) override;
		void queue_submit(QueueHandle queue_handle, const QueueSubmitDescription& queue_submit) override;
		void queue_wait_idle(QueueHandle queue_handle) override;

		WindowHandle create_window(const WindowDescription& description) override;
		TextureHandle get_window_next_texture(WindowHandle window_handle) override;
		void present_window(WindowHandle window_handle) override;
		void remove_window(WindowHandle window_handle) override;

		void wait_idle() override;

		nri::CoreInterface& get_nri_core_interface();
		nri::StreamerInterface& get_nri_streamer_interface();

		bool initialize();
		void shutdown();
	private:
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif