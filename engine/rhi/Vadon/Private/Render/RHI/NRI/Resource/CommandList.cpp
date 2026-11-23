#include <Vadon/Private/Render/RHI/NRI/Resource/CommandList.hpp>

#include <Vadon/Private/Render/RHI/NRI/Common.hpp>

#include <Vadon/Utilities/Container/Map/ConstexprMap.hpp>
#include <Vadon/Utilities/Enum/EnumBitFlagMapping.hpp>

#include <nri/NRI.h>
#include <nri/Extensions/NRIStreamer.h>

#include <array>

namespace
{
	constexpr auto c_nri_load_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::RenderPassLoadOperation, nri::LoadOp>({
		{ Vadon::Render::RHI::RenderPassLoadOperation::LOAD, nri::LoadOp::LOAD },
		{ Vadon::Render::RHI::RenderPassLoadOperation::CLEAR, nri::LoadOp::CLEAR }
	});

	constexpr auto c_nri_store_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::RenderPassStoreOperation, nri::StoreOp>({
		{ Vadon::Render::RHI::RenderPassStoreOperation::STORE, nri::StoreOp::STORE },
		{ Vadon::Render::RHI::RenderPassStoreOperation::DISCARD, nri::StoreOp::DISCARD },
		{ Vadon::Render::RHI::RenderPassStoreOperation::NONE, nri::StoreOp::NONE }
	});

	constexpr auto c_nri_access_flags_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::AccessFlags, nri::AccessBits>({
		{ Vadon::Render::RHI::AccessFlags::INDEX_BUFFER, nri::AccessBits::INDEX_BUFFER },
		{ Vadon::Render::RHI::AccessFlags::VERTEX_BUFFER, nri::AccessBits::VERTEX_BUFFER },
		{ Vadon::Render::RHI::AccessFlags::CONSTANT_BUFFER, nri::AccessBits::CONSTANT_BUFFER },
		{ Vadon::Render::RHI::AccessFlags::RENDER_TARGET_READ, nri::AccessBits::COLOR_ATTACHMENT_READ },
		{ Vadon::Render::RHI::AccessFlags::RENDER_TARGET_WRITE, nri::AccessBits::COLOR_ATTACHMENT_WRITE },
		{ Vadon::Render::RHI::AccessFlags::DEPTH_STENCIL_READ, nri::AccessBits::DEPTH_STENCIL_ATTACHMENT_READ },
		{ Vadon::Render::RHI::AccessFlags::DEPTH_STENCIL_WRITE, nri::AccessBits::DEPTH_STENCIL_ATTACHMENT_WRITE },
		{ Vadon::Render::RHI::AccessFlags::SHADER_RESOURCE, nri::AccessBits::SHADER_RESOURCE },
		{ Vadon::Render::RHI::AccessFlags::READ_WRITE_RESOURCE, nri::AccessBits::SHADER_RESOURCE_STORAGE },
		{ Vadon::Render::RHI::AccessFlags::COPY_SOURCE, nri::AccessBits::COPY_SOURCE },
		{ Vadon::Render::RHI::AccessFlags::COPY_DESTINATION, nri::AccessBits::COPY_DESTINATION },
		{ Vadon::Render::RHI::AccessFlags::HOST_READ, nri::AccessBits::HOST_READ },
		{ Vadon::Render::RHI::AccessFlags::HOST_WRITE, nri::AccessBits::HOST_WRITE }
	});

	constexpr auto c_nri_layout_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::ResourceLayout, nri::Layout>({
		{ Vadon::Render::RHI::ResourceLayout::UNDEFINED, nri::Layout::UNDEFINED },
		{ Vadon::Render::RHI::ResourceLayout::GENERAL, nri::Layout::GENERAL },
		{ Vadon::Render::RHI::ResourceLayout::PRESENT, nri::Layout::PRESENT },
		{ Vadon::Render::RHI::ResourceLayout::RENDER_TARGET, nri::Layout::COLOR_ATTACHMENT },
		{ Vadon::Render::RHI::ResourceLayout::DEPTH_STENCIL, nri::Layout::DEPTH_STENCIL_ATTACHMENT },
		{ Vadon::Render::RHI::ResourceLayout::DEPTH_READONLY_AND_STENCIL, nri::Layout::DEPTH_READONLY_STENCIL_ATTACHMENT },
		{ Vadon::Render::RHI::ResourceLayout::DEPTH_AND_STENCIL_READONLY, nri::Layout::DEPTH_ATTACHMENT_STENCIL_READONLY },
		{ Vadon::Render::RHI::ResourceLayout::DEPTH_STENCIL_READONLY, nri::Layout::DEPTH_STENCIL_READONLY },
		{ Vadon::Render::RHI::ResourceLayout::SHADER_RESOURCE, nri::Layout::SHADER_RESOURCE },
		{ Vadon::Render::RHI::ResourceLayout::READ_WRITE_RESOURCE, nri::Layout::SHADER_RESOURCE_STORAGE },
		{ Vadon::Render::RHI::ResourceLayout::COPY_SOURCE, nri::Layout::COPY_SOURCE },
		{ Vadon::Render::RHI::ResourceLayout::COPY_DESTINATION, nri::Layout::COPY_DESTINATION }
	});
	
	constexpr auto c_nri_plane_bits_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::PlaneFlags, nri::PlaneBits>({
		{ Vadon::Render::RHI::PlaneFlags::NONE, nri::PlaneBits::NONE },
		{ Vadon::Render::RHI::PlaneFlags::COLOR, nri::PlaneBits::COLOR },
		{ Vadon::Render::RHI::PlaneFlags::DEPTH, nri::PlaneBits::DEPTH },
		{ Vadon::Render::RHI::PlaneFlags::STENCIL, nri::PlaneBits::STENCIL }
	});

	nri::AttachmentDesc to_nri_attachment_desc(Vadon::Private::Render::RHI::NRI::Device& device, const Vadon::Render::RHI::RenderPassTarget& pass_target)
	{
		nri::AttachmentDesc nri_desc = {};
		nri_desc.descriptor = device.get_nri_texture_rtv(pass_target.texture);

		const Vadon::Math::ColorVector color_vector = Vadon::Math::ColorRGBA::to_rgba_vector(pass_target.clear_value.color);
		nri_desc.clearValue.color.f.x = color_vector.x;
		nri_desc.clearValue.color.f.y = color_vector.y;
		nri_desc.clearValue.color.f.z = color_vector.z;
		nri_desc.clearValue.color.f.w = color_vector.w;

		nri_desc.loadOp = c_nri_load_op_mapping.at(pass_target.load_op);
		nri_desc.storeOp = c_nri_store_op_mapping.at(pass_target.store_op);

		return nri_desc;
	}

	nri::AccessStage to_nri_access_stage(const Vadon::Render::RHI::AccessStage& access_stage)
	{
		nri::AccessStage nri_desc = {};
		nri_desc.access = c_nri_access_flags_mapping.convert(access_stage.access);
		nri_desc.stages = Vadon::Private::Render::RHI::NRI::Device::get_nri_stage_bits(access_stage.stages);

		return nri_desc;
	}

	nri::AccessLayoutStage to_nri_access_layout_stage(const Vadon::Render::RHI::AccessLayoutStage& access_layout_stage)
	{
		nri::AccessLayoutStage nri_desc = {};
		nri_desc.access = c_nri_access_flags_mapping.convert(access_layout_stage.access);
		nri_desc.layout = c_nri_layout_mapping.at(access_layout_stage.layout);
		nri_desc.stages = Vadon::Private::Render::RHI::NRI::Device::get_nri_stage_bits(access_layout_stage.stages);

		return nri_desc;
	}
}

namespace Vadon::Private::Render::RHI::NRI
{
	void CommandList::begin()
	{
		NRI_ABORT_ON_FAILURE(m_device.get_nri_core_interface().BeginCommandBuffer(*m_nri_command_buffer, nullptr));
	}

	void CommandList::end()
	{
		NRI_ABORT_ON_FAILURE(m_device.get_nri_core_interface().EndCommandBuffer(*m_nri_command_buffer));
	}

	void CommandList::begin_rendering(const RenderPassDescription& pass_description)
	{
		nri::RenderingDesc nri_desc = {};

		std::array<nri::AttachmentDesc, 8> nri_attachments;

		for(size_t target_index = 0; target_index < pass_description.targets.size(); ++target_index)
		{
			const Vadon::Render::RHI::RenderPassTarget& current_target = pass_description.targets[target_index];			
			nri_attachments[target_index] = to_nri_attachment_desc(m_device, current_target);
		}

		nri_desc.colors = nri_attachments.data();
		nri_desc.colorNum = static_cast<uint32_t>(pass_description.targets.size());
		nri_desc.depth = to_nri_attachment_desc(m_device, pass_description.depth);
		nri_desc.stencil = to_nri_attachment_desc(m_device, pass_description.stencil);

		m_device.get_nri_core_interface().CmdBeginRendering(*m_nri_command_buffer, nri_desc);
	}

	void CommandList::end_rendering()
	{
		m_device.get_nri_core_interface().CmdEndRendering(*m_nri_command_buffer);
	}

	void CommandList::set_viewports(std::span<Viewport> viewports)
	{
		std::array<nri::Viewport, 8> nri_viewports;
		for (size_t viewport_index = 0; viewport_index < viewports.size(); ++viewport_index)
		{
			const Viewport& current_viewport = viewports[viewport_index];

			nri::Viewport& nri_viewport = nri_viewports[viewport_index];
			nri_viewport.x = current_viewport.rectangle.position.x;
			nri_viewport.y = current_viewport.rectangle.position.y;
			nri_viewport.width = current_viewport.rectangle.size.x;
			nri_viewport.height = current_viewport.rectangle.size.y;
			nri_viewport.depthMin = current_viewport.depth_min_max.x;
			nri_viewport.depthMax = current_viewport.depth_min_max.y;
			nri_viewport.originBottomLeft = false;
		}

		m_device.get_nri_core_interface().CmdSetViewports(*m_nri_command_buffer, nri_viewports.data(), static_cast<uint32_t>(viewports.size()));
	}

	void CommandList::set_scissors(std::span<Vadon::Math::RectangleInt> scissor_rectangles)
	{
		std::array<nri::Rect, 8> nri_scissors;
		for (size_t rect_index = 0; rect_index < scissor_rectangles.size(); ++rect_index)
		{
			const Vadon::Math::RectangleInt& current_scissor = scissor_rectangles[rect_index];

			nri::Rect& nri_rect = nri_scissors[rect_index];
			nri_rect.x = static_cast<int16_t>(current_scissor.position.x);
			nri_rect.y = static_cast<int16_t>(current_scissor.position.y);
			nri_rect.width = static_cast<uint16_t>(current_scissor.size.x);
			nri_rect.height = static_cast<uint16_t>(current_scissor.size.y);
		}

		m_device.get_nri_core_interface().CmdSetScissors(*m_nri_command_buffer, nri_scissors.data(), static_cast<uint32_t>(scissor_rectangles.size()));
	}

	void CommandList::barrier(const BarrierDescription& description)
	{
		std::array<nri::BufferBarrierDesc, 16> nri_buffer_barriers;
		VADON_ASSERT(description.buffers.size() <= nri_buffer_barriers.size(), "Too many buffers!");

		for (size_t buffer_index = 0; buffer_index < description.buffers.size(); ++buffer_index)
		{
			const Vadon::Render::RHI::BufferBarrier& current_barrier = description.buffers[buffer_index];

			nri::BufferBarrierDesc& nri_barrier = nri_buffer_barriers[buffer_index];
			nri_barrier = {};

			nri_barrier.buffer = m_device.get_nri_buffer(current_barrier.buffer);
			nri_barrier.before = to_nri_access_stage(current_barrier.before);
			nri_barrier.after = to_nri_access_stage(current_barrier.after);
		}

		std::array<nri::TextureBarrierDesc, 16> nri_texture_barriers;
		VADON_ASSERT(description.textures.size() <= nri_texture_barriers.size(), "Too many textures!");

		for (size_t texture_index = 0; texture_index < description.textures.size(); ++texture_index)
		{
			const Vadon::Render::RHI::TextureBarrier& current_barrier = description.textures[texture_index];

			nri::TextureBarrierDesc& nri_barrier = nri_texture_barriers[texture_index];
			nri_barrier = {};

			nri_barrier.texture = m_device.get_nri_texture(current_barrier.texture);
			nri_barrier.before = to_nri_access_layout_stage(current_barrier.before);
			nri_barrier.after = to_nri_access_layout_stage(current_barrier.after);
			nri_barrier.mipOffset = current_barrier.mip_offset;
			nri_barrier.mipNum = current_barrier.mip_count;
			nri_barrier.layerOffset = current_barrier.layer_offset;
			nri_barrier.layerNum = current_barrier.layer_count;
			nri_barrier.planes = c_nri_plane_bits_mapping.convert(current_barrier.planes);
		}

		nri::BarrierDesc nri_desc = {};
		nri_desc.buffers = nri_buffer_barriers.data();
		nri_desc.bufferNum = static_cast<uint32_t>(description.buffers.size());
		nri_desc.textures = nri_texture_barriers.data();
		nri_desc.textureNum = static_cast<uint32_t>(description.textures.size());

		m_device.get_nri_core_interface().CmdBarrier(*m_nri_command_buffer, nri_desc);
	}

	void CommandList::draw(const DrawDescription& description)
	{
		nri::DrawDesc nri_desc = {};
		nri_desc.vertexNum = description.vertex_count;
		nri_desc.instanceNum = description.instance_count;
		nri_desc.baseVertex = description.base_vertex;
		nri_desc.baseInstance = description.base_instance;

		m_device.get_nri_core_interface().CmdDraw(*m_nri_command_buffer, nri_desc);
	}

	void CommandList::draw_indexed(const DrawIndexedDescription& description)
	{
		nri::DrawIndexedDesc nri_desc = {};
		nri_desc.indexNum = description.index_count;
		nri_desc.instanceNum = description.instance_count;
		nri_desc.baseIndex = description.base_index;
		nri_desc.baseVertex = description.base_vertex;
		nri_desc.baseInstance = description.base_instance;

		m_device.get_nri_core_interface().CmdDrawIndexed(*m_nri_command_buffer, nri_desc);
	}

	void CommandList::copy_streamed_data(StreamerHandle streamer, StreamerCopyBatch copy_batch)
	{
		m_device.get_nri_streamer_interface().CmdCopyStreamedData(*m_nri_command_buffer, m_device.get_nri_streamer(streamer), copy_batch);
	}
}