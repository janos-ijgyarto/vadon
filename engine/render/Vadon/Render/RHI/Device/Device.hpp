#ifndef VADON_RENDER_RHI_DEVICE_DEVICE_HPP
#define VADON_RENDER_RHI_DEVICE_DEVICE_HPP
#include <Vadon/Render/RHI/Device/Fence.hpp>
#include <Vadon/Render/RHI/Device/Queue.hpp>
#include <Vadon/Render/RHI/Device/Streamer.hpp>
#include <Vadon/Render/RHI/Device/Window.hpp>

#include <Vadon/Render/RHI/Resource/CommandList.hpp>
#include <Vadon/Render/RHI/Resource/Sampler.hpp>
#include <Vadon/Render/RHI/Resource/Shader.hpp>
namespace Vadon::Render::RHI
{
	enum class GraphicsAPI
	{
		NULL_API = 1 << 0,
		D3D12 = 1 << 1,
		VULKAN = 1 << 2,
		ALL = NULL_API | D3D12 | VULKAN
	};

	struct DeviceDescription
	{
		GraphicsAPI graphics_api = GraphicsAPI::NULL_API;
		uint32_t max_resources = 1024;
		uint32_t max_samplers = 64;
		// TODO: optional argument to specify which queue(s) to create
		bool enable_validation = false;
		bool enable_backend_validation = false;
	};

	class Device
	{
	public:
		const DeviceDescription& get_description() const { return m_description; }

		virtual BufferHandle create_buffer(const BufferDescription& description) = 0;
		virtual void remove_buffer(BufferHandle buffer_handle) = 0;

		virtual CommandAllocatorHandle create_command_allocator(QueueHandle queue_handle) = 0;
		virtual void reset_command_allocator(CommandAllocatorHandle allocator_handle) = 0;
		virtual void remove_command_allocator(CommandAllocatorHandle allocator_handle) = 0;

		virtual CommandListHandle create_command_list(CommandAllocatorHandle allocator_handle) = 0;
		virtual CommandList* get_command_list(CommandListHandle command_list_handle) = 0;
		virtual void return_command_list(CommandList* command_list) = 0;
		virtual void remove_command_list(CommandListHandle command_list_handle) = 0;

		virtual FenceHandle create_fence(const FenceDescription& description) = 0;
		virtual void wait_fence(FenceHandle fence_handle, uint64_t value) = 0;
		virtual uint64_t get_fence_value(FenceHandle fence_handle) = 0;
		virtual void remove_fence(FenceHandle fence_handle) = 0;

		virtual SamplerHandle create_sampler(const SamplerDescription& description) = 0;
		virtual void remove_sampler(SamplerHandle sampler_handle) = 0;
				
		virtual GraphicsShaderHandle create_graphics_shader(const GraphicsShaderDescription& description) = 0;
		virtual void remove_graphics_shader(GraphicsShaderHandle shader_handle) = 0;

		virtual StreamerHandle create_streamer(const StreamerDescription& description) = 0;
		virtual uint32_t stream_constant_data(StreamerHandle streamer_handle, const void* data, uint32_t data_size) = 0;
		virtual StreamerCopyBatch begin_streamer_copy_batch(StreamerHandle streamer_handle) = 0;
		virtual BufferOffset stream_buffer_data(StreamerHandle streamer_handle, const StreamBufferDataDescription& description) = 0;
		virtual BufferOffset stream_texture_data(StreamerHandle streamer_handle, const StreamTextureDataDescription& description) = 0;
		virtual void end_streamer_frame(StreamerHandle streamer_handle) = 0;
		virtual void remove_streamer(StreamerHandle streamer_handle) = 0;

		virtual TextureHandle create_texture(const TextureDescription& description) = 0;
		virtual void remove_texture(TextureHandle texture_handle) = 0;

		virtual QueueHandle get_queue(QueueType type, uint32_t index = 0) = 0;
		virtual void queue_submit(QueueHandle queue_handle, const QueueSubmitDescription& queue_submit) = 0;
		virtual void queue_wait_idle(QueueHandle queue_handle) = 0;

		virtual WindowHandle create_window(const WindowDescription& description) = 0;
		virtual TextureHandle get_window_next_texture(WindowHandle window_handle) = 0;
		virtual void present_window(WindowHandle window_handle) = 0;
		virtual void remove_window(WindowHandle window_handle) = 0;

		virtual void wait_idle() = 0;
	protected:
		Device(const DeviceDescription& description)
			: m_description(description)
		{

		}
	private:
		DeviceDescription m_description;
	};
}
#endif