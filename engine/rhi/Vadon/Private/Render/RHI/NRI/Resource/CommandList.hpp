#ifndef VADON_PRIVATE_RENDER_RHI_NRI_RESOURCE_COMMANDLIST_HPP
#define VADON_PRIVATE_RENDER_RHI_NRI_RESOURCE_COMMANDLIST_HPP
#include <Vadon/Private/Render/RHI/NRI/Device/Device.hpp>
namespace nri
{
	struct CoreInterface;
	struct CommandBuffer;
}
namespace Vadon::Private::Render::RHI::NRI
{
	using RenderPassDescription = Vadon::Render::RHI::RenderPassDescription;
	using Viewport = Vadon::Render::RHI::Viewport;
	using DrawDescription = Vadon::Render::RHI::DrawDescription;
	using DrawIndexedDescription = Vadon::Render::RHI::DrawIndexedDescription;

	class CommandList : public Vadon::Render::RHI::CommandList
	{
	public:
		CommandList(Device& device, CommandListHandle command_list_handle, nri::CommandBuffer* nri_command_buffer)
			: m_device(device)
			, m_command_list_handle(command_list_handle)
			, m_nri_command_buffer(nri_command_buffer)
		{ }

		CommandListHandle get_command_list_handle() const { return m_command_list_handle; }

		void begin() override;
		void end() override;

		void begin_rendering(const RenderPassDescription& pass_description) override;
		void end_rendering() override;

		void set_viewports(std::span<Viewport> viewports) override;
		void set_scissors(std::span<Vadon::Math::RectangleInt> scissor_rectangles) override;

		void barrier(const BarrierDescription& description) override;

		void draw(const DrawDescription& description) override;
		void draw_indexed(const DrawIndexedDescription& description) override;

		void copy_streamed_data(StreamerHandle streamer_handle, StreamerCopyBatch copy_batch) override;
	private:
		Device& m_device;
		CommandListHandle m_command_list_handle;
		nri::CommandBuffer* m_nri_command_buffer;
	};
}
#endif