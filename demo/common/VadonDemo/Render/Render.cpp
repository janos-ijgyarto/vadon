#include <VadonDemo/Render/Render.hpp>

#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Render/Component.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/ResourceSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/ShaderSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <filesystem>

namespace
{
	Vadon::Render::Canvas::LayerFlags get_layer_flags_from_def(const VadonDemo::Render::CanvasLayerDefinition& layer_def)
	{
		Vadon::Render::Canvas::LayerFlags flags = Vadon::Render::Canvas::LayerFlags::NONE;
		if (layer_def.view_agnostic == true)
		{
			flags = Vadon::Render::Canvas::LayerFlags::VIEW_AGNOSTIC;
		}

		return flags;
	}
}

namespace VadonDemo::Render
{
	void Render::register_types()
	{
		CanvasLayerDefinition::register_resource();
		TextureResource::register_resource();
		ShaderResource::register_resource();

		CanvasComponent::register_component();
		SpriteTilingComponent::register_component();
		FullscreenEffectComponent::register_component();
	}

	CanvasContextHandle Render::create_canvas_context()
	{
		return m_context_pool.add();
	}

	Vadon::Render::Canvas::RenderContext& Render::get_context(CanvasContextHandle context_handle)
	{
		return m_context_pool.get(context_handle).render_context;
	}

	void Render::init_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, CanvasContextHandle context_handle)
	{
		VADON_ASSERT(context_handle.is_valid(), "Must provide valid canvas context!"); 
		CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
		VADON_ASSERT(canvas_component != nullptr, "Entity does not have canvas component!");

		canvas_component->context_handle = context_handle;

		const Vadon::Render::Canvas::LayerHandle context_layer = get_context_layer(context_handle, canvas_component->layer_def);

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_component->canvas_item = canvas_system.create_item(Vadon::Render::Canvas::ItemInfo{ .layer = context_layer, .z_order = canvas_component->z_order });
	}
		
	void Render::update_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
		if (canvas_component == nullptr)
		{
			return;
		}

		VADON_ASSERT(canvas_component->context_handle.is_valid() == true, "Component must have valid canvas context!");
		VADON_ASSERT(canvas_component->canvas_item.is_valid() == true, "Component must have valid canvas item!");

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		
		const Vadon::Render::Canvas::LayerHandle context_layer = get_context_layer(canvas_component->context_handle, canvas_component->layer_def);
		canvas_system.set_item_layer(canvas_component->canvas_item, context_layer);

		// Update item state
		canvas_system.set_item_visible(canvas_component->canvas_item, canvas_component->visible);
		canvas_system.set_item_z_order(canvas_component->canvas_item, canvas_component->z_order);
	}

	void Render::remove_canvas_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
		if (canvas_component == nullptr)
		{
			return;
		}

		if (canvas_component->canvas_item.is_valid() == false)
		{
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.remove_item(canvas_component->canvas_item);
		canvas_component->canvas_item.invalidate();
	}

	void Render::update_layer_definition(CanvasLayerDefHandle layer_def_handle)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const CanvasLayerDefinition* layer_definition = resource_system.get_resource(layer_def_handle);

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		for (auto context_pair : m_context_pool)
		{
			CanvasContextData* current_context = context_pair.second;
			for (Vadon::Render::Canvas::LayerHandle current_context_layer : current_context->render_context.layers)
			{
				auto layer_def_it = current_context->layer_definitions.find(current_context_layer.handle.to_uint());
				VADON_ASSERT(layer_def_it != current_context->layer_definitions.end(), "Cannot find layer definition!");
				if (layer_def_it->second == layer_def_handle)
				{
					canvas_system.set_layer_flags(current_context_layer, get_layer_flags_from_def(*layer_definition));
					sort_context_layers(*current_context);
					break;
				}
			}
		}
	}

	void Render::init_sprite_tiling_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		SpriteTilingComponent* sprite_component = ecs_world.get_component_manager().get_component<SpriteTilingComponent>(entity);
		VADON_ASSERT(sprite_component, "Cannot find component!");

		// TODO: any other initialization?
		sprite_component->reset_rect();
	}

	void Render::update_sprite_tiling_entity(const CanvasComponent& canvas_component, SpriteTilingComponent& sprite_component, const Vadon::Render::Rectangle& culling_rect)
	{
		if (canvas_component.canvas_item.is_valid() == false)
		{
			return;
		}

		if (sprite_component.texture.is_valid() == false)
		{
			return;
		}

		const Vadon::Render::Vector2 extent = culling_rect.size * 0.5f;
		const Vadon::Render::Vector2 min = culling_rect.position - extent;
		const Vadon::Render::Vector2 max = culling_rect.position + extent;

		const Vadon::Render::Vector2i min_index = glm::floor(min / sprite_component.tile_size);
		const Vadon::Render::Vector2i max_index = Vadon::Render::Vector2i(glm::floor(max / sprite_component.tile_size)) + Vadon::Render::Vector2i{ 1, 1 };

		if ((sprite_component.tile_rect.position == min_index) && (sprite_component.tile_rect.size == (max_index - min_index)))
		{
			// Sprite hasn't changed
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

		canvas_system.clear_item(canvas_component.canvas_item);

		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		const TextureResource* sprite_texture = resource_system.get_resource<TextureResource>(sprite_component.texture);
		if (sprite_texture->texture.is_valid() == false)
		{
			// No valid sprite texture
			return;
		}

		canvas_system.set_item_texture(canvas_component.canvas_item, Vadon::Render::Canvas::Texture{ sprite_texture->texture_srv });

		Vadon::Render::Canvas::Sprite sprite;
		sprite.dimensions.size = sprite_component.tile_size;
		const Vadon::Render::Vector2 tile_offset = sprite_component.tile_size * 0.5f;
		for (int current_x = min_index.x; current_x < max_index.x; ++current_x)
		{
			for (int current_y = min_index.y; current_y < max_index.y; ++current_y)
			{
				sprite.dimensions.position = Vadon::Render::Vector2(current_x, current_y) * sprite_component.tile_size + tile_offset;
				canvas_system.draw_item_sprite(canvas_component.canvas_item, sprite);
			}
		}

		sprite_component.tile_rect.position = min_index;
		sprite_component.tile_rect.size = max_index - min_index;
	}

	void Render::remove_sprite_tiling_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		// TODO: reference counting for resources so we unload a texture once nothing is referencing it
		CanvasComponent* canvas_component = ecs_world.get_component_manager().get_component<CanvasComponent>(entity);
		if (canvas_component != nullptr)
		{
			Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
			Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();

			canvas_system.clear_item(canvas_component->canvas_item);
		}
	}

	void Render::init_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		FullscreenEffectComponent* fullscreen_component = ecs_world.get_component_manager().get_component<FullscreenEffectComponent>(entity);
		VADON_ASSERT(fullscreen_component != nullptr, "Entity does not have fullscreen component!");

		// TODO: anything?
	}

	void Render::update_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		FullscreenEffectComponent* fullscreen_component = ecs_world.get_component_manager().get_component<FullscreenEffectComponent>(entity);
		if (fullscreen_component == nullptr)
		{
			return;
		}

		VADON_ASSERT(fullscreen_component->shader.is_valid() == true, "Component must have valid shader!");

		// TODO: anything to do?
	}

	void Render::remove_fullscreen_effect_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		FullscreenEffectComponent* fullscreen_component = ecs_world.get_component_manager().get_component<FullscreenEffectComponent>(entity);
		if (fullscreen_component == nullptr)
		{
			return;
		}

		if (fullscreen_component->shader.is_valid() == false)
		{
			return;
		}

		// TODO: implement refcounting where if the shader is no longer referenced, we unload it
	}

	bool Render::init_texture_resource(TextureResourceHandle texture_handle, Vadon::Core::RootDirectoryHandle root_dir) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		TextureResource* texture_resource = resource_system.get_resource<TextureResource>(texture_handle);
		if(texture_resource->texture.is_valid() == true)
		{ 
			// Texture already loaded
			return true;
		}

		if (texture_resource->file_path.empty() == true)
		{
			// Nothing to do
			return false;
		}

		// FIXME: accept other extensions!
		std::filesystem::path texture_fs_path = texture_resource->file_path;
		texture_fs_path.replace_extension(".dds");

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystemPath file_path{ .root_directory = root_dir, .path = texture_fs_path.string() };

		TextureResource resource_data;

		if (file_system.does_file_exist(file_path) == false)
		{
			return false;
		}

		Vadon::Core::FileSystem::RawFileDataBuffer texture_file_buffer;
		if (file_system.load_file(file_path, texture_file_buffer) == false)
		{
			return false;
		}

		Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();
		Vadon::Render::TextureHandle render_texture_handle = texture_system.create_texture_from_memory(texture_file_buffer.size(), texture_file_buffer.data());
		if (texture_handle.is_valid() == false)
		{
			return false;
		}

		// Create texture view
		// FIXME: we are guessing the texture is compatible, should have a way to verify it!
		const Vadon::Render::TextureInfo texture_info = texture_system.get_texture_info(render_texture_handle);

		Vadon::Render::TextureSRVInfo texture_srv_info;
		texture_srv_info.format = texture_info.format;
		texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
		texture_srv_info.mip_levels = texture_info.mip_levels;
		texture_srv_info.most_detailed_mip = 0;

		Vadon::Render::SRVHandle srv_handle = texture_system.create_shader_resource_view(render_texture_handle, texture_srv_info);
		if (srv_handle.is_valid() == false)
		{
			texture_system.remove_texture(render_texture_handle);
			return false;
		}

		texture_resource->texture = render_texture_handle;
		texture_resource->texture_srv = srv_handle;
		return true;
	}

	void Render::unload_texture_resource(TextureResourceHandle texture_handle) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		TextureResource* texture_resource = resource_system.get_resource<TextureResource>(texture_handle);
		if (texture_resource->texture.is_valid() == false)
		{
			// Nothing to do
			return;
		}

		// Unload previous texture
		Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();
		texture_system.remove_texture(texture_resource->texture);
		texture_resource->texture.invalidate();

		engine_core.get_system<Vadon::Render::ResourceSystem>().remove_srv(texture_resource->texture_srv);
		texture_resource->texture_srv.invalidate();
	}

	bool Render::init_shader_resource(ShaderResourceHandle shader_handle, Vadon::Core::RootDirectoryHandle root_dir) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		ShaderResource* shader_resource = resource_system.get_resource<ShaderResource>(shader_handle);
		if (shader_resource->pixel_shader.is_valid() == true)
		{
			// Nothing to do
			return true;
		}

		// FIXME: accept other extensions!
		std::filesystem::path shader_fs_path = shader_resource->shader_path;
		shader_fs_path.replace_extension(".hlsl");

		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystemPath file_path{ .root_directory = root_dir, .path = shader_fs_path.string() };

		if (file_system.does_file_exist(file_path) == false)
		{
			return false;
		}

		Vadon::Core::FileSystem::RawFileDataBuffer shader_file_buffer;
		if (file_system.load_file(file_path, shader_file_buffer) == false)
		{
			return false;
		}

		Vadon::Render::ShaderInfo shader_info;

		// TODO: at the moment we always expect a pixel shader in this exact format
		// Need to extend API to support shader files in general!
		shader_info.name = shader_fs_path.stem().string();
		shader_info.entrypoint = "ps_main";
		shader_info.type = Vadon::Render::ShaderType::PIXEL;
		shader_info.source = reinterpret_cast<const char*>(shader_file_buffer.data());

		Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
		shader_resource->pixel_shader = shader_system.create_shader(shader_info);
		if (shader_resource->pixel_shader.is_valid() == false)
		{
			return false;
		}

		return true;
	}

	void Render::unload_shader_resource(ShaderResourceHandle shader_handle) const
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();

		ShaderResource* shader_resource = resource_system.get_resource<ShaderResource>(shader_handle);
		if (shader_resource->pixel_shader.is_valid() == false)
		{
			// Nothing to do
			return;
		}

		// Unload previous shader
		Vadon::Render::ShaderSystem& shader_system = engine_core.get_system<Vadon::Render::ShaderSystem>();
		shader_system.remove_shader(shader_resource->pixel_shader);
		shader_resource->pixel_shader.invalidate();
	}

	Render::Render(VadonDemo::Core::Core& core)
		: m_core(core)
	{

	}

	bool Render::initialize()
	{
		// TODO: anything?
		return true;
	}

	void Render::global_config_updated()
	{
		// TODO: anything?
	}

	Vadon::Render::Canvas::LayerHandle Render::get_context_layer(CanvasContextHandle context_handle, CanvasLayerDefHandle layer_def_handle)
	{
		if (layer_def_handle.is_valid() == false)
		{
			return Vadon::Render::Canvas::LayerHandle();
		}

		CanvasContextData& context_data = m_context_pool.get(context_handle);
		VADON_ASSERT(context_data.layer_definitions.size() == context_data.render_context.layers.size(), "Mismatch between layers and definitions!");
		for (const auto& layer_pair : context_data.layer_definitions)
		{
			if (layer_def_handle == layer_pair.second)
			{
				Vadon::Render::Canvas::LayerHandle layer_handle;
				layer_handle.handle.from_uint(layer_pair.first);
				return layer_handle;
			}
		}

		// Layer not in context, add it
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		const CanvasLayerDefinition* layer_definition = resource_system.get_resource(layer_def_handle);

		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		const Vadon::Render::Canvas::LayerFlags layer_flags = get_layer_flags_from_def(*layer_definition);

		Vadon::Render::Canvas::LayerHandle new_layer_handle = canvas_system.create_layer(Vadon::Render::Canvas::LayerInfo{ .flags = layer_flags });
		context_data.layer_definitions[new_layer_handle.handle.to_uint()] = layer_def_handle;
		context_data.render_context.layers.push_back(new_layer_handle);

		sort_context_layers(context_data);

		return new_layer_handle;
	}

	void Render::sort_context_layers(CanvasContextData& context)
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Scene::ResourceSystem& resource_system = engine_core.get_system<Vadon::Scene::ResourceSystem>();
		std::sort(context.render_context.layers.begin(), context.render_context.layers.end(),
			[&context, &resource_system](const Vadon::Render::Canvas::LayerHandle& lhs, const Vadon::Render::Canvas::LayerHandle& rhs)
			{
				auto lhs_def_it = context.layer_definitions.find(lhs.handle.to_uint());
				VADON_ASSERT(lhs_def_it != context.layer_definitions.end(), "Cannot find layer definition!");
				auto rhs_def_it = context.layer_definitions.find(rhs.handle.to_uint());
				VADON_ASSERT(rhs_def_it != context.layer_definitions.end(), "Cannot find layer definition!");

				const CanvasLayerDefinition* lhs_definition = resource_system.get_resource(lhs_def_it->second);
				const CanvasLayerDefinition* rhs_definition = resource_system.get_resource(rhs_def_it->second);

				// Lower priority should go first, since last to be drawn is placed on top
				return lhs_definition->priority < rhs_definition->priority;
			}
		);
	}
}