#include <Vadon/Private/Render/Canvas/CanvasSystem.hpp>

#include <Vadon/Render/Canvas/Context.hpp>

#include <Vadon/Render/GraphicsAPI/Buffer/BufferSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Pipeline/PipelineSystem.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTargetSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Render/Utilities/Projection.hpp>

#include <Vadon/Private/Render/Utilities/Matrix.hpp>
#include <Vadon/Private/Render/Utilities/Rectangle.hpp>
#include <Vadon/Private/Render/Utilities/Vector.hpp>

#include <Vadon/Private/Render/Canvas/Shader.hpp>

namespace Vadon::Private::Render::Canvas
{
	namespace
	{
		struct alignas(16) ShaderCanvasView
		{
			Matrix4 view_projection = Matrix4(1.0f);
			Matrix4 projection = Matrix4(1.0f); // Passing in projection without view for view-agnostic layers
			// TODO: any other data?
		};

		struct alignas(16) ShaderCanvasLayer
		{
			Vector2 offset = Vadon::Render::Vector2_Zero;
			float scale = 1.0f;
			uint32_t flags = 0;
			// TODO: any other info?

			void initialize(const LayerData& data)
			{
				offset = data.info.transform.position;
				scale = data.info.transform.scale;
				flags = Vadon::Utilities::to_integral(data.info.flags);
				// TODO: other data?
			}
		};

		// Cap the layer count at the number we can store in a constant buffer
		constexpr size_t c_max_layer_data_count = 4096;

		// FIXME: make this dynamic? Or just cap the material count?
		constexpr size_t c_max_material_count = 8192;

		struct alignas(16) ShaderCanvasMaterial
		{
			// TODO: any other material data?
			struct alignas(16) SDF
			{
				uint32_t flags = 0;
				float edge_distance = 0.1f;
				float edge_width_factor = 0.7f;
				uint32_t _padding = 0;
			};

			SDF sdf;

			void initialize(const MaterialData& material)
			{
				sdf.edge_distance = material.sdf.edge_distance;
				sdf.edge_width_factor = material.sdf.edge_width_factor;
				sdf.flags = Vadon::Utilities::to_integral(material.sdf.flags);

				// TODO: other data?
			}
		};

		constexpr size_t c_primitive_offset_width = 24;

		constexpr int32_t get_primitive_index_count(PrimitiveType type)
		{
			switch (type)
			{
			case PrimitiveType::TRIANGLE:
				return 3;
			case PrimitiveType::RECTANGLE_FILL:
				return 6;
			case PrimitiveType::RECTANGLE_OUTLINE:
				return 24;
			}

			return 0;
		}

		constexpr uint32_t get_primitive_vertex_index(PrimitiveType type, int32_t index)
		{
			switch (type)
			{
			case PrimitiveType::TRIANGLE:
			{
				constexpr std::array<uint32_t, get_primitive_index_count(PrimitiveType::TRIANGLE)> c_triangle_indices{ 0, 1, 2 };
				return c_triangle_indices[index];
			}
			case PrimitiveType::RECTANGLE_FILL:
			{
				constexpr std::array<uint32_t, get_primitive_index_count(PrimitiveType::RECTANGLE_FILL)> c_rect_indices{ 0, 1, 2, 1, 3, 2 };
				return c_rect_indices[index];
			}
			case PrimitiveType::RECTANGLE_OUTLINE:
			{
				constexpr std::array<uint32_t, get_primitive_index_count(PrimitiveType::RECTANGLE_OUTLINE)> c_rect_indices
				{
					0, 1, 4, 4, 1, 5, // Top
					0, 4, 2, 2, 4, 6, // Left
					1, 3, 7, 5, 1, 7, // Right
					6, 7, 2, 2, 7, 3 // Bottom
				};
				return c_rect_indices[index];
			}
			}

			return 0;
		}

		uint32_t get_primitive_info(const ItemData& item, PrimitiveType type)
		{
			return PrimitiveInfo{ .type = type, .layer = item.info.layer.handle.index }.to_index();
		}

		using ItemTriangleCommand = ItemDirectDrawCommand<Triangle, BatchCommandType::TRIANGLE>;
		using ItemRectangleCommand = ItemDirectDrawCommand<Rectangle, BatchCommandType::RECTANGLE>;
		using ItemSpriteCommand = ItemDirectDrawCommand<Sprite, BatchCommandType::SPRITE>;
	}

	void CanvasSystem::FrameData::add_command(PrimitiveType primitive_type, Vadon::Render::SRVHandle texture_srv)
	{
		const int32_t index_count = get_primitive_index_count(primitive_type);

		if (primitive_batches.empty() == false)
		{
			FrameData::PrimitiveBatch& prev_batch = primitive_batches.back();
			if (prev_batch.texture_srv == texture_srv)
			{
				// Can just append to previous batch
				prev_batch.index_count += index_count;
				add_primitive_indices(primitive_type);
				return;
			}
		}

		// Add new batch
		FrameData::PrimitiveBatch& new_batch = primitive_batches.emplace_back();
		new_batch.index_count = index_count;
		new_batch.texture_srv = texture_srv;

		add_primitive_indices(primitive_type);
	}

	void CanvasSystem::FrameData::add_primitive_indices(PrimitiveType primitive_type)
	{
		const int32_t index_count = get_primitive_index_count(primitive_type);
		const uint32_t data_offset = static_cast<uint32_t>(primitive_data.size());

		// Add indices
		// Lower 24 bits used for byte offset, highest 8 bits used for vertex index
		// All indices will reference the same data offset
		for (int32_t current_index = 0; current_index < index_count; ++current_index)
		{
			// Add the vertex index for the primitive
			indices.push_back(data_offset | (get_primitive_vertex_index(primitive_type, current_index) << 24));
		}
	}

	LayerHandle CanvasSystem::create_layer(LayerInfo info)
	{
		LayerHandle new_layer_handle = m_layer_pool.add();

		LayerData& new_layer = m_layer_pool.get(new_layer_handle);
		new_layer.info = info;

		m_shared_data.add_dirty_layer(new_layer_handle);

		return new_layer_handle;
	}

	LayerInfo CanvasSystem::get_layer_info(LayerHandle layer_handle) const
	{
		const LayerData& layer = m_layer_pool.get(layer_handle);
		return layer.info;
	}

	void CanvasSystem::remove_layer(LayerHandle layer_handle)
	{
		// TODO: any cleanup?
		m_layer_pool.remove(layer_handle);
	}

	void CanvasSystem::set_layer_transform(LayerHandle layer_handle, const Transform& transform)
	{
		LayerData& layer = m_layer_pool.get(layer_handle);
		layer.info.transform = transform;

		m_shared_data.add_dirty_layer(layer_handle);
	}

	void CanvasSystem::set_layer_flags(LayerHandle layer_handle, LayerInfo::Flags flags)
	{
		LayerData& layer = m_layer_pool.get(layer_handle);
		layer.info.flags = flags;

		m_shared_data.add_dirty_layer(layer_handle);
	}

	ItemHandle CanvasSystem::create_item(ItemInfo info)
	{
		ItemHandle new_item_handle = m_item_pool.add();

		// TODO: move this to Layer implementation!
		LayerData& layer = m_layer_pool.get(info.layer);
		layer.items.push_back(new_item_handle);

		layer.set_items_dirty();

		ItemData& new_item = m_item_pool.get(new_item_handle);
		new_item.info = info;

		return new_item_handle;
	}

	ItemInfo CanvasSystem::get_item_info(ItemHandle item_handle) const
	{
		const ItemData& item = m_item_pool.get(item_handle);
		return item.info;
	}

	void CanvasSystem::remove_item(ItemHandle item_handle)
	{
		const ItemData& item = m_item_pool.get(item_handle);

		if (item.info.layer.is_valid() == true)
		{
			// Remove from Layer
			// TODO: move this to Layer implementation!
			LayerData& layer = m_layer_pool.get(item.info.layer);
			layer.items.erase(std::remove(layer.items.begin(), layer.items.end(), item_handle), layer.items.end());
			
			layer.set_items_dirty();
		}

		// TODO: any other cleanup?
		m_item_pool.remove(item_handle);
	}

	void CanvasSystem::set_item_visible(ItemHandle item_handle, bool visible)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.visible = visible;

		set_item_layer_dirty(item);
	}

	void CanvasSystem::set_item_transform(ItemHandle item_handle, const Transform& transform)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.info.transform = transform;
	}

	void CanvasSystem::set_item_z_order(ItemHandle item_handle, float z_order)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.info.z_order = z_order;

		set_item_layer_dirty(item);
	}

	size_t CanvasSystem::get_item_buffer_size(ItemHandle item_handle) const
	{
		const ItemData& item = m_item_pool.get(item_handle);
		return item.command_buffer.get_size();
	}

	void CanvasSystem::clear_item(ItemHandle item_handle)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.command_buffer.clear();
	}

	void CanvasSystem::add_item_batch_draw(ItemHandle item_handle, const BatchDrawCommand& batch_command)
	{
		VADON_ASSERT(batch_command.range.is_valid() == true, "Invalid batch command!");
		VADON_ASSERT(batch_command.range.count > 0, "Invalid batch command!");

		ItemData& item = m_item_pool.get(item_handle);
		item.command_buffer.write_object<BatchDrawCommand>(Vadon::Utilities::to_integral(ItemCommandType::DRAW_BATCH), batch_command);
	}

	void CanvasSystem::draw_item_triangle(ItemHandle item_handle, const Triangle& triangle)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.command_buffer.write_object(Vadon::Utilities::to_integral(ItemCommandType::DRAW_DIRECT), ItemTriangleCommand(triangle));
	}

	void CanvasSystem::draw_item_rectangle(ItemHandle item_handle, const Rectangle& rectangle)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.command_buffer.write_object(Vadon::Utilities::to_integral(ItemCommandType::DRAW_DIRECT), ItemRectangleCommand(rectangle));
	}

	void CanvasSystem::draw_item_sprite(ItemHandle item_handle, const Sprite& sprite)
	{
		ItemData& item = m_item_pool.get(item_handle);
		item.command_buffer.write_object(Vadon::Utilities::to_integral(ItemCommandType::DRAW_DIRECT), ItemSpriteCommand(sprite));
	}

	MaterialHandle CanvasSystem::create_material(MaterialInfo info)
	{
		MaterialHandle new_material_handle = m_material_pool.add();

		MaterialData& new_material = m_material_pool.get(new_material_handle);
		new_material.info = info;

		m_shared_data.add_dirty_material(new_material_handle);

		return new_material_handle;
	}

	MaterialInfo CanvasSystem::get_material_info(MaterialHandle material_handle) const
	{
		return m_material_pool.get(material_handle).info;
	}

	void CanvasSystem::remove_material(MaterialHandle material_handle)
	{
		// TODO: any cleanup?
		m_material_pool.remove(material_handle);
	}

	void CanvasSystem::set_material_sdf(MaterialHandle material_handle, SDFParameters parameters)
	{
		MaterialData& material = m_material_pool.get(material_handle);
		material.sdf = parameters;

		m_shared_data.add_dirty_material(material_handle);
	}

	BatchHandle CanvasSystem::create_batch()
	{
		// TODO: anything else?
		return m_batch_pool.add();
	}

	void CanvasSystem::remove_batch(BatchHandle batch_handle)
	{
		// TODO: any cleanup?
		m_batch_pool.remove(batch_handle);
	}

	size_t CanvasSystem::get_batch_buffer_size(BatchHandle batch_handle) const
	{
		const BatchData& batch = m_batch_pool.get(batch_handle);
		return batch.command_buffer.get_size();
	}

	void CanvasSystem::clear_batch(BatchHandle batch_handle)
	{
		BatchData& batch = m_batch_pool.get(batch_handle);
		batch.command_buffer.clear();
	}

	void CanvasSystem::draw_batch_triangle(BatchHandle batch_handle, const Triangle& triangle)
	{
		BatchData& batch = m_batch_pool.get(batch_handle);
		batch.command_buffer.write_object(Vadon::Utilities::to_integral(BatchCommandType::TRIANGLE), triangle);
	}

	void CanvasSystem::draw_batch_rectangle(BatchHandle batch_handle, const Rectangle& rectangle)
	{
		BatchData& batch = m_batch_pool.get(batch_handle);
		batch.command_buffer.write_object(Vadon::Utilities::to_integral(BatchCommandType::RECTANGLE), rectangle);
	}

	void CanvasSystem::draw_batch_sprite(BatchHandle batch_handle, const Sprite& sprite)
	{
		BatchData& batch = m_batch_pool.get(batch_handle);
		batch.command_buffer.write_object(Vadon::Utilities::to_integral(BatchCommandType::SPRITE), sprite);
	}

	void CanvasSystem::render(const RenderContext& context)
	{
		update_shared_data();
		update_frame_data(context);

		init_render_state(context);

		Vadon::Render::GraphicsAPI& graphics_api = m_engine_core.get_system<Vadon::Render::GraphicsAPI>();
		Vadon::Render::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Render::ResourceSystem>();

		int32_t index_offset = 0;
		for (const FrameData::PrimitiveBatch& current_batch : m_frame_data.primitive_batches)
		{
			// Apply the texture
			Vadon::Render::SRVHandle current_texture_srv = (current_batch.texture_srv.is_valid() == true) ? current_batch.texture_srv : m_default_texture_srv;
			resource_system.apply_shader_resource(Vadon::Render::ShaderType::PIXEL, current_texture_srv, 2);

			// Draw the batch
			const Vadon::Render::DrawCommand current_draw_command {
					.type = Vadon::Render::DrawCommandType::INDEXED,
					.vertices = Vadon::Utilities::DataRange(),
					.indices = Vadon::Utilities::DataRange{ .offset = index_offset, .count = current_batch.index_count }
			};
			graphics_api.draw(current_draw_command);

			// Update index offset
			index_offset += current_batch.index_count;
		}
	}

	bool CanvasSystem::initialize()
	{
		// Create pipeline state
		{
			Vadon::Render::PipelineSystem& pipeline_system = m_engine_core.get_system<Vadon::Render::PipelineSystem>();

			// Add alpha-blending
			{
				Vadon::Render::BlendInfo blend_info;
				Vadon::Render::RenderTargetBlendInfo& rt_blend_info = blend_info.render_target[0];

				// Make a disabled blend state
				rt_blend_info.blend_enable = false;
				rt_blend_info.write_mask = Vadon::Render::ColorWriteEnable::ALL;
				m_blend_disabled_state = pipeline_system.get_blend_state(blend_info);

				if (m_blend_disabled_state.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}

				// Make alpha blend state for translucent canvas items
				rt_blend_info.blend_enable = true;
				rt_blend_info.source_blend = Vadon::Render::BlendType::SRC_ALPHA;
				rt_blend_info.dest_blend = Vadon::Render::BlendType::INV_SRC_ALPHA;
				rt_blend_info.blend_operation = Vadon::Render::BlendOperation::ADD;
				rt_blend_info.source_blend_alpha = Vadon::Render::BlendType::ONE;
				rt_blend_info.dest_blend_alpha = Vadon::Render::BlendType::INV_SRC_ALPHA;
				rt_blend_info.blend_operation_alpha = Vadon::Render::BlendOperation::ADD;
				rt_blend_info.write_mask = Vadon::Render::ColorWriteEnable::ALL;

				m_alpha_blend_state = pipeline_system.get_blend_state(blend_info);
				if (m_alpha_blend_state.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}

				m_pipeline_state.blend_update.blend_state = m_blend_disabled_state;
				m_pipeline_state.blend_update.blend_factor.fill(1.0f);
			}

			// Add depth
			{
				Vadon::Render::DepthStencilInfo depth_stencil_info;

				Vadon::Render::DepthInfo& depth_info = depth_stencil_info.depth;

				// Create disabled depth state (won't be using depth buffer for now)
				depth_info.enable = false;
				depth_info.write_mask = Vadon::Render::DepthWriteMask::ALL;
				depth_info.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::LESS;

				Vadon::Render::StencilInfo& stencil_info = depth_stencil_info.stencil;

				stencil_info.enable = false;
				stencil_info.front_face.fail = stencil_info.front_face.depth_fail = stencil_info.front_face.pass = Vadon::Render::StencilOperation::KEEP;
				stencil_info.front_face.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;
				stencil_info.back_face = stencil_info.front_face;

				m_pipeline_state.depth_stencil_update.depth_stencil = pipeline_system.get_depth_stencil_state(depth_stencil_info);

				if (m_pipeline_state.depth_stencil_update.depth_stencil.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}

				m_pipeline_state.depth_stencil_update.stencil_ref = 0;
			}
		}

		// Create shaders
		Vadon::Render::ShaderSystem& shader_system = m_engine_core.get_system<Vadon::Render::ShaderSystem>();

		{
			// NOTE: no need for vertex layout, we procedurally generate polygons in the shader
			Vadon::Render::ShaderInfo vertex_shader_info;
			vertex_shader_info.source = Shader::c_shader_source;
			vertex_shader_info.entrypoint = "vs_main";
			vertex_shader_info.name = "CanvasVShader";
			vertex_shader_info.type = Vadon::Render::ShaderType::VERTEX;

			m_shader.vertex_shader = shader_system.create_shader(vertex_shader_info);
			if (m_shader.vertex_shader.is_valid() == false)
			{
				// TODO: error message?
				return false;
			}
		}

		{
			Vadon::Render::ShaderInfo pixel_shader_info;
			pixel_shader_info.source = Shader::c_shader_source;
			pixel_shader_info.entrypoint = "ps_main";
			pixel_shader_info.name = "CanvasPShader";
			pixel_shader_info.type = Vadon::Render::ShaderType::PIXEL;

			m_shader.pixel_shader = shader_system.create_shader(pixel_shader_info);
			if (m_shader.pixel_shader.is_valid() == false)
			{
				// TODO: error message?
				return false;
			}
		}

		// Set up buffers
		Vadon::Render::BufferSystem& buffer_system = m_engine_core.get_system<Vadon::Render::BufferSystem>();
		{
			// Materials
			// FIXME: could make this a constant buffer like the layers?
			{
				Vadon::Render::BufferInfo material_buffer_info;
				material_buffer_info.type = Vadon::Render::BufferType::STRUCTURED;
				material_buffer_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
				material_buffer_info.element_size = sizeof(ShaderCanvasMaterial);
				material_buffer_info.capacity = c_max_material_count;

				m_material_buffer = buffer_system.create_buffer(material_buffer_info);

				if (m_material_buffer.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}

				Vadon::Render::BufferSRVInfo buffer_srv_info;
				buffer_srv_info.format = Vadon::Render::GraphicsAPIDataFormat::UNKNOWN;
				buffer_srv_info.first_element = 0;
				buffer_srv_info.num_elements = c_max_material_count;

				m_material_buffer_srv = buffer_system.create_buffer_srv(m_material_buffer, buffer_srv_info);

				if (m_material_buffer_srv.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}
			}

			// Primitive data
			{
				m_primitive_buffer_info.type = Vadon::Render::BufferType::STRUCTURED;
				m_primitive_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
				m_primitive_buffer_info.element_size = sizeof(Vector4);
				m_primitive_buffer_info.capacity = 1024;

				m_primitive_buffer = buffer_system.create_buffer(m_primitive_buffer_info);

				if (m_primitive_buffer.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}

				m_primitive_buffer_srv_info.format = Vadon::Render::GraphicsAPIDataFormat::UNKNOWN;
				m_primitive_buffer_srv_info.first_element = 0;
				m_primitive_buffer_srv_info.num_elements = 1024;

				m_primitive_buffer_srv = buffer_system.create_buffer_srv(m_primitive_buffer, m_primitive_buffer_srv_info);

				if (m_primitive_buffer_srv.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}
			}

			// Indices
			{
				m_index_buffer_info.type = Vadon::Render::BufferType::INDEX;
				m_index_buffer_info.usage = Vadon::Render::ResourceUsage::DYNAMIC;
				m_index_buffer_info.element_size = sizeof(uint32_t);
				m_index_buffer_info.capacity = 1024;

				m_index_buffer = buffer_system.create_buffer(m_index_buffer_info);

				if (m_index_buffer.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}
			}
		}

		// Create constant buffers
		{
			// View
			{
				Vadon::Render::BufferInfo view_cbuffer_info;
				view_cbuffer_info.type = Vadon::Render::BufferType::CONSTANT;
				view_cbuffer_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
				view_cbuffer_info.element_size = sizeof(ShaderCanvasView);
				view_cbuffer_info.capacity = 1;

				constexpr ShaderCanvasView init_view;
				m_view_cbuffer = buffer_system.create_buffer(view_cbuffer_info, &init_view);
			}

			// Layers
			{
				Vadon::Render::BufferInfo buffer_info;
				buffer_info.type = Vadon::Render::BufferType::CONSTANT;
				buffer_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
				buffer_info.element_size = sizeof(ShaderCanvasLayer);
				buffer_info.capacity = static_cast<int32_t>(c_max_layer_data_count);

				m_layers_cbuffer = buffer_system.create_buffer(buffer_info);
				if (m_layers_cbuffer.is_valid() == false)
				{
					// TODO: error message?
					return false;
				}
			}
		}

		// Create default texture
		Vadon::Render::TextureSystem& texture_system = m_engine_core.get_system<Vadon::Render::TextureSystem>();
		{
			Vadon::Render::TextureInfo default_texture_info;
			default_texture_info.dimensions.x = 1;
			default_texture_info.dimensions.y = 1;
			default_texture_info.mip_levels = 1;
			default_texture_info.array_size = 1;
			default_texture_info.format = Vadon::Render::GraphicsAPIDataFormat::R32G32B32A32_FLOAT;
			default_texture_info.sample_info.count = 1;
			default_texture_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
			default_texture_info.flags = Vadon::Render::TextureFlags::SHADER_RESOURCE;

			constexpr Render::Vector4 c_placeholder_data{ 1.0f, 1.0f, 1.0f, 1.0f };
			m_default_texture = texture_system.create_texture(default_texture_info, &c_placeholder_data);
			if (m_default_texture.is_valid() == false)
			{
				log_error("Canvas system: failed to create default texture!\n");
				return false;
			}

			// Create texture view
			Vadon::Render::TextureSRVInfo texture_srv_info;
			texture_srv_info.format = Vadon::Render::GraphicsAPIDataFormat::R32G32B32A32_FLOAT;
			texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
			texture_srv_info.mip_levels = default_texture_info.mip_levels;
			texture_srv_info.most_detailed_mip = 0;

			m_default_texture_srv = texture_system.create_shader_resource_view(m_default_texture, texture_srv_info);
			if (m_default_texture_srv.is_valid() == false)
			{
				log_error("Canvas system: failed to create default texture SRV!\n");
				return false;
			}
		}

		// Prepare sampler
		{
			Vadon::Render::TextureSamplerInfo sampler_info;
			sampler_info.filter = Vadon::Render::TextureFilter::MIN_MAG_MIP_LINEAR;
			sampler_info.address_u = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.address_v = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.address_w = Vadon::Render::TextureAddressMode::WRAP;
			sampler_info.mip_lod_bias = 0.0f;
			sampler_info.comparison_func = Vadon::Render::GraphicsAPIComparisonFunction::ALWAYS;
			sampler_info.min_lod = 0.0f;
			sampler_info.max_lod = 0.0f;

			m_sampler = texture_system.create_sampler(sampler_info);
		}

		// Add default material
		{
			MaterialInfo default_material_info
			{
				.name = "_DefaultMaterial"
			};

			m_default_material = create_material(default_material_info);
			// TODO: set SDF?
			// TODO2: default text material?
		}

		return true;
	}

	void CanvasSystem::init_render_state(const RenderContext& context)
	{
		Vadon::Render::BufferSystem& buffer_system = m_engine_core.get_system<Vadon::Render::BufferSystem>();
		Vadon::Render::PipelineSystem& pipeline_system = m_engine_core.get_system<Vadon::Render::PipelineSystem>();

		pipeline_system.apply_blend_state(m_pipeline_state.blend_update);
		pipeline_system.apply_depth_stencil_state(m_pipeline_state.depth_stencil_update);
		pipeline_system.apply_rasterizer_state(m_pipeline_state.rasterizer_state);

		pipeline_system.set_primitive_topology(Vadon::Render::PrimitiveTopology::TRIANGLE_LIST);

		// Set render target and viewport
		// FIXME: render to multiple targets?
		{
			const Viewport& viewport = context.viewports.front();

			// Apply RT
			Vadon::Render::RenderTargetSystem& rt_system = m_engine_core.get_system<Vadon::Render::RenderTargetSystem>();
			rt_system.set_target(viewport.render_target, Vadon::Render::DSVHandle());

			// Apply viewport
			rt_system.apply_viewport(viewport.render_viewport);
		}

		Vadon::Render::ShaderSystem& shader_system = m_engine_core.get_system<Vadon::Render::ShaderSystem>();
		shader_system.apply_shader(m_shader.vertex_shader);
		shader_system.apply_shader(m_shader.pixel_shader);
		shader_system.set_vertex_layout(m_shader.vertex_layout);

		// Set constant buffers
		{
			Vadon::Render::BufferHandle constant_buffers[] = { m_view_cbuffer, m_layers_cbuffer };
			buffer_system.set_constant_buffer_slots(Vadon::Render::ShaderType::VERTEX, { .start_slot = 0, .buffers = constant_buffers });
		}

		// Update view buffer
		// FIXME: only update when it changes?
		{
			constexpr Vector2 c_projection_near_far = { -100.0f, 100.0f };
			ShaderCanvasView canvas_view;

			const Vector2 bottom_left = -(context.camera.view_rectangle.size * 0.5f);
			const Vector2 top_right = context.camera.view_rectangle.size * 0.5f;

			canvas_view.projection = Vadon::Render::create_directx_orthographic_projection_matrix(bottom_left.x, top_right.x, bottom_left.y, top_right.y, c_projection_near_far.x, c_projection_near_far.y, Vadon::Render::CoordinateSystem::RIGHT_HAND);
			{
				const Matrix4 view = {
					context.camera.zoom, 0, 0, -context.camera.view_rectangle.position.x * context.camera.zoom,
					0, context.camera.zoom, 0, -context.camera.view_rectangle.position.y * context.camera.zoom,
					0, 0, 1, 0,
					0, 0, 0, 1
				};

				canvas_view.view_projection = view * canvas_view.projection;
			}

			buffer_system.buffer_data(m_view_cbuffer,
				Vadon::Render::BufferWriteData{
					.range = Vadon::Utilities::DataRange{ .offset = 0, .count = 1 },
					.data = &canvas_view
				});
		}

		// Set index buffer
		buffer_system.set_index_buffer(m_index_buffer, Vadon::Render::GraphicsAPIDataFormat::R32_UINT);

		// Set resources
		Vadon::Render::ResourceSystem& resource_system = m_engine_core.get_system<Vadon::Render::ResourceSystem>();
		{
			Vadon::Render::SRVHandle vertex_shader_resources[] = { m_primitive_buffer_srv };
			resource_system.apply_shader_resource_slots(Vadon::Render::ShaderType::VERTEX, { .start_slot = 1, .resources = vertex_shader_resources });
		}
		{
			Vadon::Render::SRVHandle pixel_shader_resources[] = { m_material_buffer_srv, m_primitive_buffer_srv };
			resource_system.apply_shader_resource_slots(Vadon::Render::ShaderType::PIXEL, { .start_slot = 0, .resources = pixel_shader_resources });
		}

		// Set sampler
		Vadon::Render::TextureSystem& texture_system = m_engine_core.get_system<Vadon::Render::TextureSystem>();
		texture_system.set_sampler(Vadon::Render::ShaderType::PIXEL, m_sampler, 2);
	}

	void CanvasSystem::update_shared_data()
	{
		// FIXME: use an event queue system instead?
		update_layers();
		update_materials();
	}

	void CanvasSystem::update_frame_data(const RenderContext& context)
	{
		// Reset frame buffers
		m_frame_data.clear();

		constexpr static auto parse_batch_command = +[](CanvasSystem& canvas_system, const ItemData& item_data, const Transform& transform, BatchCommandType command_type, const std::byte* command_data) {
			// FIXME: transforms are currently computed on CPU
			// Could shift all of this work to GPU?
			// Either we generate primitives at final position, or we can make it so we only upload the variable data (either transform or primitives)
			switch (command_type)
			{
			case BatchCommandType::TRIANGLE:
			{
				const Triangle& triangle = *reinterpret_cast<const Triangle*>(command_data);

				// Add command
				canvas_system.m_frame_data.add_command(PrimitiveType::TRIANGLE, Vadon::Render::SRVHandle());

				// Generate primitive data, offset with position
				TrianglePrimitiveData triangle_primitive;
				triangle_primitive.info = get_primitive_info(item_data, PrimitiveType::TRIANGLE);
				triangle_primitive.material = canvas_system.get_material_index(triangle);
				triangle_primitive.point_a = PrimitiveVertex{ .position = (triangle.point_a.position * transform.scale) + transform.position, .uv = triangle.point_a.uv };
				triangle_primitive.point_b = PrimitiveVertex{ .position = (triangle.point_b.position * transform.scale) + transform.position, .uv = triangle.point_b.uv };
				triangle_primitive.point_c = PrimitiveVertex{ .position = (triangle.point_c.position * transform.scale) + transform.position, .uv = triangle.point_c.uv };
				triangle_primitive.depth = 0.0f; // TODO: depth?
				triangle_primitive.color = triangle.color;

				canvas_system.m_frame_data.add_primitive_data(triangle_primitive);
			}
			break;
			case BatchCommandType::RECTANGLE:
			{
				const Rectangle& rectangle = *reinterpret_cast<const Rectangle*>(command_data);

				// Add command
				const PrimitiveType rectangle_type = rectangle.filled ? PrimitiveType::RECTANGLE_FILL : PrimitiveType::RECTANGLE_OUTLINE;
				canvas_system.m_frame_data.add_command(rectangle_type, Vadon::Render::SRVHandle());

				RectanglePrimitiveData rectangle_primitive;
				rectangle_primitive.info = get_primitive_info(item_data, rectangle_type);
				rectangle_primitive.material = canvas_system.get_material_index(rectangle);
				rectangle_primitive.dimensions.position = (rectangle.dimensions.position * transform.scale) + transform.position;
				rectangle_primitive.dimensions.size = rectangle.dimensions.size * transform.scale;
				rectangle_primitive.uvs[0] = {0, 0};
				rectangle_primitive.uvs[1] = { 1, 0 };
				rectangle_primitive.uvs[2] = { 0, 1 };
				rectangle_primitive.uvs[3] = { 1, 1 };
				rectangle_primitive.depth = 0.0f; // TODO: depth?
				rectangle_primitive.thickness = rectangle.thickness;
				rectangle_primitive.color = rectangle.color;

				canvas_system.m_frame_data.add_primitive_data(rectangle_primitive);
			}
			break;
			case BatchCommandType::SPRITE:
			{
				const Sprite& sprite = *reinterpret_cast<const Sprite*>(command_data);

				canvas_system.m_frame_data.add_command(PrimitiveType::RECTANGLE_FILL, sprite.texture_view_handle);

				RectanglePrimitiveData rectangle_primitive;
				rectangle_primitive.info = get_primitive_info(item_data, PrimitiveType::RECTANGLE_FILL);
				rectangle_primitive.material = canvas_system.get_material_index(sprite);
				rectangle_primitive.dimensions.position = (sprite.dimensions.position * transform.scale) + transform.position;
				rectangle_primitive.dimensions.size = sprite.dimensions.size * transform.scale;
				rectangle_primitive.uvs[0] = sprite.uv_top_left;
				rectangle_primitive.uvs[1] = sprite.uv_top_right;
				rectangle_primitive.uvs[2] = sprite.uv_bottom_left;
				rectangle_primitive.uvs[3] = sprite.uv_bottom_right;
				rectangle_primitive.depth = 0.0f; // TODO: depth?
				rectangle_primitive.thickness = 0.0f;
				rectangle_primitive.color = sprite.color;

				canvas_system.m_frame_data.add_primitive_data(rectangle_primitive);
			}
			break;
			}
		};

		constexpr static auto draw_batch = +[](CanvasSystem& canvas_system, const ItemData& item_data, const BatchDrawCommand& batch_draw, const BatchData& batch_data)
			{
				const Transform batch_transform = Transform::combine(item_data.info.transform, batch_draw.transform);
				BatchCommandBuffer::Iterator command_it = batch_data.command_buffer.get_iterator(batch_draw.range.offset);

				int32_t command_count = 0;
				while ((command_it.is_valid() == true) && (command_count < batch_draw.range.count))
				{
					parse_batch_command(canvas_system, item_data, batch_transform, Vadon::Utilities::to_enum<BatchCommandType>(command_it.get_header().packet_id), command_it.get_packet_data());
					command_it.advance();
					++command_count;
				}
			};

		// Gather frame data
		PrimitiveInfo primitive_info;
		// FIXME: some kind of ordering for layers? Or assume provided order is intended?
		for (LayerHandle layer_handle : context.layers)
		{
			LayerData& current_layer_data = m_layer_pool.get(layer_handle);
			update_layer_items(current_layer_data);

			// FIXME: should sort primitives based on layer and Z order, for now just do them in whatever order they are in
			for (size_t current_item_index = 0; current_item_index < current_layer_data.visible_count; ++current_item_index)
			{
				const ItemData& current_item_data = m_item_pool.get(current_layer_data.items[current_item_index]);
				if (current_item_data.command_buffer.is_empty() == true)
				{
					continue;
				}
				
				ItemCommandBuffer::Iterator item_command_it = current_item_data.command_buffer.get_iterator();
				for(item_command_it; item_command_it.is_valid() == true; item_command_it.advance())
				{
					const ItemCommandType current_command_type = Vadon::Utilities::to_enum<ItemCommandType>(item_command_it.get_header().packet_id);
					switch (current_command_type)
					{
					case ItemCommandType::DRAW_BATCH:
					{
						const BatchDrawCommand* batch_draw = reinterpret_cast<const BatchDrawCommand*>(item_command_it.get_packet_data());
						const BatchData& batch_data = m_batch_pool.get(batch_draw->batch);
						draw_batch(*this, current_item_data, *batch_draw, batch_data);
					}
					break;
					case ItemCommandType::DRAW_DIRECT:
					{
						const std::byte* batch_command_ptr = item_command_it.get_packet_data();
						// First value is the command type
						const uint32_t command_type_int = *reinterpret_cast<const uint32_t*>(batch_command_ptr);
						parse_batch_command(*this, current_item_data, current_item_data.info.transform, Vadon::Utilities::to_enum<BatchCommandType>(command_type_int), batch_command_ptr + sizeof(uint32_t));
					}
					break;
					}
				}
			}
		}

		// All frame data gathered, update buffers
		buffer_frame_data();
	}

	void CanvasSystem::update_layers()
	{
		if (m_shared_data.dirty_layers.empty() == true)
		{
			// Nothing to do
			return;
		}

		Vadon::Render::BufferSystem& buffer_system = m_engine_core.get_system<Vadon::Render::BufferSystem>();

		ShaderCanvasLayer canvas_layer_data;

		Vadon::Render::BufferWriteData write_data;
		write_data.range.count = 1;
		write_data.data = &canvas_layer_data;
		write_data.no_overwrite = true;

		for (LayerHandle layer_handle : m_shared_data.dirty_layers)
		{
			write_data.range.offset = layer_handle.handle.index;

			const LayerData& dirty_layer = m_layer_pool.get(layer_handle);
			canvas_layer_data.initialize(dirty_layer);

			buffer_system.buffer_data(m_layers_cbuffer, write_data);
		}

		m_shared_data.dirty_layers.clear();
	}

	void CanvasSystem::update_materials()
	{
		if (m_shared_data.dirty_materials.empty() == true)
		{
			// Nothing to do
			return;
		}

		Vadon::Render::BufferSystem& buffer_system = m_engine_core.get_system<Vadon::Render::BufferSystem>();

		ShaderCanvasMaterial canvas_material;

		Vadon::Render::BufferWriteData write_data;
		write_data.range.count = 1;
		write_data.data = &canvas_material;

		for (MaterialHandle material_handle : m_shared_data.dirty_materials)
		{
			write_data.range.offset = material_handle.handle.index;

			const MaterialData& dirty_material = m_material_pool.get(material_handle);
			canvas_material.initialize(dirty_material);

			buffer_system.buffer_data(m_material_buffer, write_data);
		}

		m_shared_data.dirty_materials.clear();
	}

	void CanvasSystem::buffer_frame_data()
	{
		// FIXME: should try using NO_OVERWRITE to save on buffer reallocation?
		Vadon::Render::BufferSystem& buffer_system = m_engine_core.get_system<Vadon::Render::BufferSystem>();

		// Buffer primitive data
		{
			// FIXME: utility system that handles this buffer bookkeeping for us!
			const int32_t total_primitive_data_count = static_cast<int32_t>(m_frame_data.primitive_data.size());

			if (total_primitive_data_count > m_primitive_buffer_info.capacity)
			{
				// Need to increase the primitive buffer capacity
				m_primitive_buffer_info.capacity = total_primitive_data_count;
				m_primitive_buffer_srv_info.num_elements = total_primitive_data_count;

				if (m_primitive_buffer.is_valid() == true)
				{
					buffer_system.remove_buffer(m_primitive_buffer);
					m_engine_core.get_system<Vadon::Render::ResourceSystem>().remove_srv(m_primitive_buffer_srv);
				}

				m_primitive_buffer = buffer_system.create_buffer(m_primitive_buffer_info, m_frame_data.primitive_data.data());
				m_primitive_buffer_srv = buffer_system.create_buffer_srv(m_primitive_buffer, m_primitive_buffer_srv_info);
			}
			else
			{
				buffer_system.buffer_data(m_primitive_buffer, { .range = {.offset = 0, .count = total_primitive_data_count }, .data = m_frame_data.primitive_data.data() });
			}
		}

		// Buffer indices
		{
			// FIXME: utility system that handles this buffer bookkeeping for us!
			const int32_t total_index_count = static_cast<int32_t>(m_frame_data.indices.size());

			if (total_index_count > m_index_buffer_info.capacity)
			{
				// Need to increase the primitive buffer capacity
				m_index_buffer_info.capacity = total_index_count;

				if (m_index_buffer.is_valid())
				{
					buffer_system.remove_buffer(m_index_buffer);
				}

				m_index_buffer = buffer_system.create_buffer(m_index_buffer_info, m_frame_data.indices.data());
			}
			else
			{
				buffer_system.buffer_data(m_index_buffer, { .range = {.offset = 0, .count = total_index_count }, .data = m_frame_data.indices.data() });
			}
		}
	}

	uint32_t CanvasSystem::get_material_index(const PrimitiveBase& primitive)
	{
		return primitive.material.is_valid() == true ? primitive.material.handle.index : m_default_material.handle.index;
	}

	void CanvasSystem::set_item_layer_dirty(const ItemData& item)
	{
		if (item.info.layer.is_valid() == true)
		{
			LayerData& layer = m_layer_pool.get(item.info.layer);
			layer.set_items_dirty();
		}
	}

	void CanvasSystem::update_layer_items(LayerData& layer)
	{
		if (layer.items_dirty == false)
		{
			return;
		}

		layer.items_dirty = false;

		std::sort(layer.items.begin(), layer.items.end(),
			[this](ItemHandle lhs, ItemHandle rhs)
			{
				const ItemData& left_item = m_item_pool.get(lhs);
				const ItemData& right_item = m_item_pool.get(rhs);

				if ((left_item.visible == true) && (right_item.visible == false))
				{
					return true;
				}

				return left_item.info.z_order < right_item.info.z_order;
			}
		);

		layer.visible_count = 0;
		for (ItemHandle current_item_handle : layer.items)
		{
			const ItemData& current_item = m_item_pool.get(current_item_handle);
			if (current_item.visible == false)
			{
				break;
			}
			++layer.visible_count;
		}
	}
}