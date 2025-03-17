#include <VadonDemo/Render/Render.hpp>

#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/Render/Component.hpp>

#include <Vadon/Core/File/FileSystem.hpp>
#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <filesystem>

namespace
{
	Vadon::Render::Canvas::LayerInfo::Flags get_layer_flags_from_def(const VadonDemo::Render::CanvasLayerDefinition& layer_def)
	{
		Vadon::Render::Canvas::LayerInfo::Flags flags = Vadon::Render::Canvas::LayerInfo::Flags::NONE;
		if (layer_def.view_agnostic == true)
		{
			flags = Vadon::Render::Canvas::LayerInfo::Flags::VIEW_AGNOSTIC;
		}

		return flags;
	}
}

namespace VadonDemo::Render
{
	void Render::register_types()
	{
		CanvasLayerDefinition::register_resource();

		CanvasComponent::register_component();
	}

	CanvasContextHandle Render::create_canvas_context()
	{
		return m_context_pool.add();
	}

	Vadon::Render::Canvas::RenderContext& Render::get_context(CanvasContextHandle context_handle)
	{
		return m_context_pool.get(context_handle).render_context;
	}

	void Render::init_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity, CanvasContextHandle context_handle)
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
		
	void Render::update_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
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

	void Render::remove_entity(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
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

	TextureResource Render::load_texture_resource(const Vadon::Core::FileSystemPath& path) const
	{
		// FIXME: accept other extensions!
		std::filesystem::path texture_fs_path = path.path;
		texture_fs_path.replace_extension(".dds");

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Core::FileSystem& file_system = engine_core.get_system<Vadon::Core::FileSystem>();
		Vadon::Core::FileSystemPath file_path{ .root_directory = path.root_directory, .path = texture_fs_path.string() };

		TextureResource resource_data;

		if (file_system.does_file_exist(file_path) == false)
		{
			return resource_data;
		}

		Vadon::Core::FileSystem::RawFileDataBuffer texture_file_buffer;
		if (file_system.load_file(file_path, texture_file_buffer) == false)
		{
			return resource_data;
		}

		Vadon::Render::TextureSystem& texture_system = engine_core.get_system<Vadon::Render::TextureSystem>();
		Vadon::Render::TextureHandle texture_handle = texture_system.create_texture_from_memory(texture_file_buffer.size(), texture_file_buffer.data());
		if (texture_handle.is_valid() == false)
		{
			return resource_data;
		}

		const Vadon::Render::TextureInfo texture_info = texture_system.get_texture_info(texture_handle);

		// Create texture view
		// FIXME: we are guessing the texture is compatible, should have a way to verify it!
		Vadon::Render::TextureSRVInfo texture_srv_info;
		texture_srv_info.format = texture_info.format;
		texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
		texture_srv_info.mip_levels = texture_info.mip_levels;
		texture_srv_info.most_detailed_mip = 0;

		Vadon::Render::SRVHandle srv_handle = texture_system.create_shader_resource_view(texture_handle, texture_srv_info);
		if (srv_handle.is_valid() == false)
		{
			texture_system.remove_texture(texture_handle);
			return resource_data;
		}

		resource_data.texture = texture_handle;
		resource_data.srv = srv_handle;

		return resource_data;
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
		const Vadon::Render::Canvas::LayerInfo::Flags layer_flags = get_layer_flags_from_def(*layer_definition);

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