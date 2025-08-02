#include <Vadon/Private/PCH/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Pipeline/PipelineSystem.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/GraphicsAPI.hpp>

#include <Vadon/Utilities/Enum/EnumClass.hpp>

namespace Vadon::Private::Render::DirectX
{
	namespace
	{
		constexpr D3D11_BLEND get_d3d_blend_type(BlendType blend_type)
		{
			switch (blend_type)
			{
			case BlendType::ZERO:
				return D3D11_BLEND_ZERO;
			case BlendType::ONE:
				return D3D11_BLEND_ONE;
			case BlendType::SRC_COLOR:
				return D3D11_BLEND_SRC_COLOR;
			case BlendType::INV_SRC_COLOR:
				return D3D11_BLEND_INV_SRC_COLOR;
			case BlendType::SRC_ALPHA:
				return D3D11_BLEND_SRC_ALPHA;
			case BlendType::INV_SRC_ALPHA:
				return D3D11_BLEND_INV_SRC_ALPHA;
			case BlendType::DEST_ALPHA:
				return D3D11_BLEND_DEST_ALPHA;
			case BlendType::INV_DEST_ALPHA:
				return D3D11_BLEND_INV_DEST_ALPHA;
			case BlendType::DEST_COLOR:
				return D3D11_BLEND_DEST_COLOR;
			case BlendType::INV_DEST_COLOR:
				return D3D11_BLEND_INV_DEST_COLOR;
			case BlendType::SRC_ALPHA_SAT:
				return D3D11_BLEND_SRC_ALPHA_SAT;
			case BlendType::BLEND_FACTOR:
				return D3D11_BLEND_BLEND_FACTOR;
			case BlendType::INV_BLEND_FACTOR:
				return D3D11_BLEND_INV_BLEND_FACTOR;
			case BlendType::SRC1_COLOR:
				return D3D11_BLEND_SRC1_COLOR;
			case BlendType::INV_SRC1_COLOR:
				return D3D11_BLEND_INV_SRC1_COLOR;
			case BlendType::SRC1_ALPHA:
				return D3D11_BLEND_SRC1_ALPHA;
			case BlendType::INV_SRC1_ALPHA:
				return D3D11_BLEND_INV_SRC1_ALPHA;
			}

			return D3D11_BLEND_ZERO;
		}

		constexpr D3D11_BLEND_OP get_d3d_blend_op(BlendOperation blend_op)
		{
			switch (blend_op)
			{
			case BlendOperation::ADD:
				return D3D11_BLEND_OP_ADD;
			case BlendOperation::SUBTRACT:
				return D3D11_BLEND_OP_SUBTRACT;
			case BlendOperation::REV_SUBTRACT:
				return D3D11_BLEND_OP_REV_SUBTRACT;
			case BlendOperation::MIN:
				return D3D11_BLEND_OP_MIN;
			case BlendOperation::MAX:
				return D3D11_BLEND_OP_MAX;
			}

			return D3D11_BLEND_OP_ADD;
		}

		constexpr D3D11_PRIMITIVE_TOPOLOGY get_d3d_primitive_topology(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::UNDEFINED:
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			case PrimitiveTopology::LINE_LIST:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::TRIANGLE_LIST:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			}

			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		constexpr D3D11_FILL_MODE get_d3d_fill_mode(RasterizerFillMode fill_mode)
		{
			switch (fill_mode)
			{
			case RasterizerFillMode::WIREFRAME:
				return D3D11_FILL_WIREFRAME;
			case RasterizerFillMode::SOLID:
				return D3D11_FILL_SOLID;
			}

			return D3D11_FILL_SOLID;
		}

		constexpr D3D11_CULL_MODE get_d3d_cull_mode(RasterizerCullMode cull_mode)
		{
			switch (cull_mode)
			{
			case RasterizerCullMode::NONE:
				return D3D11_CULL_NONE;
			case RasterizerCullMode::FRONT:
				return D3D11_CULL_FRONT;
			case RasterizerCullMode::BACK:
				return D3D11_CULL_BACK;
			}

			return D3D11_CULL_NONE;
		}

		constexpr D3D11_DEPTH_WRITE_MASK get_d3d_depth_write_mask(DepthWriteMask mask)
		{
			switch (mask)
			{
			case DepthWriteMask::ZERO:
				return D3D11_DEPTH_WRITE_MASK_ZERO;
			case DepthWriteMask::ALL:
				return D3D11_DEPTH_WRITE_MASK_ALL;
			}

			return D3D11_DEPTH_WRITE_MASK_ZERO;
		}

		constexpr D3D11_STENCIL_OP get_d3d_stencil_op(StencilOperation stencil_op)
		{
			switch (stencil_op)
			{
			case StencilOperation::KEEP:
				return D3D11_STENCIL_OP_KEEP;
			case StencilOperation::ZERO:
				return D3D11_STENCIL_OP_ZERO;
			case StencilOperation::REPLACE:
				return D3D11_STENCIL_OP_REPLACE;
			case StencilOperation::INCR_SAT:
				return D3D11_STENCIL_OP_INCR_SAT;
			case StencilOperation::DECR_SAT:
				return D3D11_STENCIL_OP_DECR_SAT;
			case StencilOperation::INVERT:
				return D3D11_STENCIL_OP_INVERT;
			case StencilOperation::INCR:
				return D3D11_STENCIL_OP_INCR;
			case StencilOperation::DECR:
				return D3D11_STENCIL_OP_DECR;
			}

			return D3D11_STENCIL_OP_KEEP;
		}

		D3D11_DEPTH_STENCILOP_DESC get_d3d_depth_stencil_op_desc(const StencilOperationInfo& stencil_op_info)
		{
			D3D11_DEPTH_STENCILOP_DESC d3d_depth_stencilop_desc;
			ZeroMemory(&d3d_depth_stencilop_desc, sizeof(D3D11_DEPTH_STENCILOP_DESC));

			d3d_depth_stencilop_desc.StencilFailOp = get_d3d_stencil_op(stencil_op_info.fail);
			d3d_depth_stencilop_desc.StencilDepthFailOp = get_d3d_stencil_op(stencil_op_info.depth_fail);
			d3d_depth_stencilop_desc.StencilPassOp = get_d3d_stencil_op(stencil_op_info.pass);
			d3d_depth_stencilop_desc.StencilFunc = get_d3d_comparison_func(stencil_op_info.comparison_func);

			return d3d_depth_stencilop_desc;
		}
	}

	BlendStateHandle PipelineSystem::get_blend_state(const BlendInfo& blend_info)
	{
		// FIXME: check other blend states to make sure we haven't added an equal one previously
		BlendStateHandle new_blend_state_handle;

		D3D11_BLEND_DESC blend_description;
		ZeroMemory(&blend_description, sizeof(D3D11_BLEND_DESC));

		blend_description.AlphaToCoverageEnable = blend_info.alpha_to_coverage_enable;
		blend_description.IndependentBlendEnable = blend_info.independent_blend_enable;

		D3D11_RENDER_TARGET_BLEND_DESC* current_d3d_rt_blend_info = blend_description.RenderTarget;
		for (const RenderTargetBlendInfo& current_rt_blend_info : blend_info.render_target)
		{
			current_d3d_rt_blend_info->BlendEnable = current_rt_blend_info.blend_enable;
			current_d3d_rt_blend_info->SrcBlend = get_d3d_blend_type(current_rt_blend_info.source_blend);
			current_d3d_rt_blend_info->DestBlend = get_d3d_blend_type(current_rt_blend_info.dest_blend);
			current_d3d_rt_blend_info->BlendOp = get_d3d_blend_op(current_rt_blend_info.blend_operation);
			current_d3d_rt_blend_info->SrcBlendAlpha = get_d3d_blend_type(current_rt_blend_info.source_blend_alpha);
			current_d3d_rt_blend_info->DestBlendAlpha = get_d3d_blend_type(current_rt_blend_info.dest_blend_alpha);
			current_d3d_rt_blend_info->BlendOpAlpha = get_d3d_blend_op(current_rt_blend_info.blend_operation_alpha);
			current_d3d_rt_blend_info->RenderTargetWriteMask = Vadon::Utilities::to_integral(current_rt_blend_info.write_mask);

			++current_d3d_rt_blend_info;
		}

		D3DBlendState new_d3d_blend_state;
		GraphicsAPI::Device* device = m_graphics_api.get_device();

		const HRESULT result = device->CreateBlendState(&blend_description, new_d3d_blend_state.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error message!
			return new_blend_state_handle;
		}

		new_blend_state_handle = m_blend_state_pool.add();

		BlendState& new_blend_state = m_blend_state_pool.get(new_blend_state_handle);
		new_blend_state.info = blend_info;
		new_blend_state.d3d_blend_state = new_d3d_blend_state;

		return new_blend_state_handle;
	}

	void PipelineSystem::apply_blend_state(const BlendStateUpdate& blend_update)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		if (blend_update.blend_state.is_valid() == false)
		{
			// Use default blend state
			device_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
			return;
		}

		const BlendState& blend_state = m_blend_state_pool.get(blend_update.blend_state);
		device_context->OMSetBlendState(blend_state.d3d_blend_state.Get(), blend_update.blend_factor.data(), blend_update.sample_mask);
	}

	void PipelineSystem::remove_blend_state(BlendStateHandle blend_handle)
	{
		BlendState& blend_state = m_blend_state_pool.get(blend_handle);

		blend_state.d3d_blend_state.Reset();

		m_blend_state_pool.remove(blend_handle);
	}

	RasterizerStateHandle PipelineSystem::get_rasterizer_state(const RasterizerInfo& rasterizer_info)
	{
		// FIXME: check other rasterizer states to make sure we haven't added an equal one previously
		RasterizerStateHandle new_rasterizer_handle;

		D3D11_RASTERIZER_DESC rasterizer_description;
		ZeroMemory(&rasterizer_description, sizeof(D3D11_RASTERIZER_DESC));

		rasterizer_description.FillMode = get_d3d_fill_mode(rasterizer_info.fill_mode);
		rasterizer_description.CullMode = get_d3d_cull_mode(rasterizer_info.cull_mode);
		rasterizer_description.FrontCounterClockwise = rasterizer_info.front_counter_clockwise;
		rasterizer_description.DepthBias = rasterizer_info.depth_bias;
		rasterizer_description.DepthBiasClamp = rasterizer_info.depth_bias_clamp;
		rasterizer_description.SlopeScaledDepthBias = rasterizer_info.slope_scaled_depth_bias;
		rasterizer_description.DepthClipEnable = rasterizer_info.depth_clip_enable;
		rasterizer_description.ScissorEnable = rasterizer_info.scissor_enable;
		rasterizer_description.MultisampleEnable = rasterizer_info.multisample_enable;
		rasterizer_description.AntialiasedLineEnable = rasterizer_info.antialiased_line_enable;

		D3DRasterizerState new_d3d_rasterizer_state;
		GraphicsAPI::Device* device = m_graphics_api.get_device();

		const HRESULT result = device->CreateRasterizerState(&rasterizer_description, new_d3d_rasterizer_state.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error message!
			return new_rasterizer_handle;
		}

		new_rasterizer_handle = m_rasterizer_state_pool.add();

		RasterizerState& new_rasterizer_state = m_rasterizer_state_pool.get(new_rasterizer_handle);
		new_rasterizer_state.info = rasterizer_info;
		new_rasterizer_state.d3d_rasterizer_state = new_d3d_rasterizer_state;

		return new_rasterizer_handle;
	}

	void PipelineSystem::apply_rasterizer_state(RasterizerStateHandle rasterizer_handle)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		if (!rasterizer_handle.is_valid())
		{
			// Use default rasterizer state
			device_context->RSSetState(nullptr);
			return;
		}

		const RasterizerState& rasterizer_state = m_rasterizer_state_pool.get(rasterizer_handle);
		device_context->RSSetState(rasterizer_state.d3d_rasterizer_state.Get());
	}

	void PipelineSystem::remove_rasterizer_state(RasterizerStateHandle rasterizer_handle)
	{
		RasterizerState& rasterizer_state = m_rasterizer_state_pool.get(rasterizer_handle);
		rasterizer_state.d3d_rasterizer_state.Reset();

		m_rasterizer_state_pool.remove(rasterizer_handle);
	}

	DepthStencilStateHandle PipelineSystem::get_depth_stencil_state(const DepthStencilInfo& depth_stencil_info)
	{
		// FIXME: check other depth-stencil states to make sure we haven't added an equal one previously
		DepthStencilStateHandle new_depth_stencil_handle;

		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		ZeroMemory(&depth_stencil_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		const DepthInfo& depth_info = depth_stencil_info.depth;
		
		depth_stencil_desc.DepthEnable = depth_info.enable;
		depth_stencil_desc.DepthWriteMask = get_d3d_depth_write_mask(depth_info.write_mask);
		depth_stencil_desc.DepthFunc = get_d3d_comparison_func(depth_info.comparison_func);

		const StencilInfo& stencil_info = depth_stencil_info.stencil;

		depth_stencil_desc.StencilEnable = stencil_info.enable;
		depth_stencil_desc.StencilReadMask = stencil_info.read_mask;
		depth_stencil_desc.StencilWriteMask = stencil_info.write_mask;

		depth_stencil_desc.FrontFace = get_d3d_depth_stencil_op_desc(stencil_info.front_face);
		depth_stencil_desc.BackFace = get_d3d_depth_stencil_op_desc(stencil_info.back_face);

		D3DDepthStencilState new_d3d_depth_stencil_state;
		GraphicsAPI::Device* device = m_graphics_api.get_device();

		const HRESULT result = device->CreateDepthStencilState(&depth_stencil_desc, new_d3d_depth_stencil_state.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			// TODO: error message!
			return new_depth_stencil_handle;
		}

		new_depth_stencil_handle = m_depth_stencil_state_pool.add();

		DepthStencilState& new_depth_stencil_state = m_depth_stencil_state_pool.get(new_depth_stencil_handle);
		new_depth_stencil_state.info = depth_stencil_info;
		new_depth_stencil_state.d3d_depth_stencil_state = new_d3d_depth_stencil_state;

		return new_depth_stencil_handle;
	}

	void PipelineSystem::apply_depth_stencil_state(const DepthStencilUpdate& depth_stencil_update)
	{
		GraphicsAPI::DeviceContext* device_context = m_graphics_api.get_device_context();
		if (!depth_stencil_update.depth_stencil.is_valid())
		{
			// Use default depth-stencil state
			device_context->OMSetDepthStencilState(nullptr, 0);
			return;
		}

		const DepthStencilState& depth_stencil_state = m_depth_stencil_state_pool.get(depth_stencil_update.depth_stencil);
		device_context->OMSetDepthStencilState(depth_stencil_state.d3d_depth_stencil_state.Get(), depth_stencil_update.stencil_ref);
	}

	void PipelineSystem::remove_depth_stencil_state(DepthStencilStateHandle depth_stencil_handle)
	{
		DepthStencilState& depth_stencil_state = m_depth_stencil_state_pool.get(depth_stencil_handle);
		depth_stencil_state.d3d_depth_stencil_state.Reset();

		m_depth_stencil_state_pool.remove(depth_stencil_handle);
	}

	void PipelineSystem::set_primitive_topology(PrimitiveTopology topology)
	{
		const D3D11_PRIMITIVE_TOPOLOGY d3d_topology = get_d3d_primitive_topology(topology);
		m_graphics_api.get_device_context()->IASetPrimitiveTopology(d3d_topology);
	}

	void PipelineSystem::set_scissor(const Math::RectangleInt& scissor_rectangle)
	{
		const D3D11_RECT d3d_rect = {
			(LONG)scissor_rectangle.position.x,
			(LONG)scissor_rectangle.position.y,
			(LONG)scissor_rectangle.position.x + scissor_rectangle.size.x,
			(LONG)scissor_rectangle.position.y + scissor_rectangle.size.y
		};

		// FIXME: allow more options
		m_graphics_api.get_device_context()->RSSetScissorRects(1, &d3d_rect);
	}

	PipelineSystem::PipelineSystem(Core::EngineCoreInterface& core, GraphicsAPI& graphics_api)
		: Vadon::Render::PipelineSystem(core)
		, m_graphics_api(graphics_api)
	{}

	bool PipelineSystem::initialize()
	{
		// TODO!!!
		return true;
	}

	void PipelineSystem::shutdown()
	{
		log_message("Shutting down Pipeline system (DirectX).\n");

		// Clear pools
		// TODO: add warning in case of leftover resources?
		m_blend_state_pool.reset();
		m_rasterizer_state_pool.reset();
		m_depth_stencil_state_pool.reset();

		log_message("Pipeline system (DirectX) shut down successfully.\n");
	}
}