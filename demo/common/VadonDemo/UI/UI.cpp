#include <VadonDemo/UI/UI.hpp>

#include <VadonDemo/UI/Component.hpp>
#include <VadonDemo/UI/DefaultFont.inl>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/Text/TextSystem.hpp>

namespace VadonDemo::UI
{
	struct UI::Internal
	{
		Vadon::Core::EngineCoreInterface& m_engine_core;

		Vadon::Render::FontHandle m_default_font;
		Vadon::Render::Canvas::MaterialHandle m_text_sdf_material;

		Internal(Vadon::Core::EngineCoreInterface& engine_core)
			: m_engine_core(engine_core)
		{ }

		bool initialize()
		{
			Base::register_component();
			Frame::register_component();
			Text::register_component();
			Selectable::register_component();

			if (load_default_font() == false)
			{
				return false;
			}

			{
				Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();				 
				m_text_sdf_material = canvas_system.create_material(Vadon::Render::Canvas::MaterialInfo{ .name = "default_text_sdf" });
				if (m_text_sdf_material.is_valid() == false)
				{
					return false;
				}
				canvas_system.set_material_sdf(m_text_sdf_material, Vadon::Render::Canvas::SDFParameters{ .flags = Vadon::Render::Canvas::SDFParameters::Flags::ENABLED });
			}

			return true;
		}

		// TODO: manage fonts via resources!
		bool load_default_font()
		{
			Vadon::Render::TextSystem& text_system = m_engine_core.get_system<Vadon::Render::TextSystem>();

			Vadon::Render::FontInfo default_font_info;
			default_font_info.height = 32;

			m_default_font = text_system.create_font(default_font_info, std::span(c_default_font, sizeof(c_default_font)));
			if (m_default_font.is_valid() == false)
			{
				// TODO: error!
				return false;
			}

			return true;
		}

		void update(Vadon::ECS::World& ecs_world)
		{
			update_selectables(ecs_world);
		}

		void update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			
			auto component_tuple = component_manager.get_component_tuple<Base, Frame, Text, Selectable>(entity);
			Base* base_component = std::get<Base*>(component_tuple);
			if (base_component == nullptr)
			{
				// TODO: error?
				return;
			}
			VADON_ASSERT(base_component->canvas_item.is_valid() == true, "Canvas item not created for UI element!");

			Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			canvas_system.clear_item(base_component->canvas_item);

			{
				Vadon::Render::Canvas::Transform transform;
				transform.position = base_component->position;

				canvas_system.set_item_transform(base_component->canvas_item, transform);
				canvas_system.set_item_z_order(base_component->canvas_item, base_component->z_order);
			}

			const Frame* frame_component = std::get<Frame*>(component_tuple);
			if (frame_component != nullptr)
			{
				// NOTE: offsetting position so we are drawing from the top left corner
				Vadon::Render::Canvas::Rectangle frame_rect;
				frame_rect.dimensions.size = base_component->dimensions;
				frame_rect.dimensions.position = Vadon::Utilities::Vector2(base_component->dimensions.x, -base_component->dimensions.y) * 0.5f;
				frame_rect.color = Vadon::Render::Canvas::ColorRGBA(frame_component->outline_color, 1.0f);
				frame_rect.filled = false;

				canvas_system.draw_item_rectangle(base_component->canvas_item, frame_rect);
			}

			const Text* text_component = std::get<Text*>(component_tuple);
			if ((text_component != nullptr) && (text_component->text.empty() == false))
			{
				Vadon::Render::TextSystem& text_system = m_engine_core.get_system<Vadon::Render::TextSystem>();

				Vadon::Render::TextRenderInfo text_info;
				text_info.font = m_default_font;
				text_info.scale = 1.0f;
				text_info.width = base_component->dimensions.x;

				Vadon::Render::TextRenderData text_render_data = text_system.create_text_render_data(text_component->text, text_info);

				Vadon::Render::Canvas::Sprite glyph_sprite;
				glyph_sprite.material = m_text_sdf_material;
				glyph_sprite.texture_view_handle = text_render_data.font_texture_view;
				glyph_sprite.color = Vadon::Render::Canvas::ColorRGBA(text_component->color, 1.0f);

				for (Vadon::Render::TextGlyph& current_glyph : text_render_data.glyphs)
				{
					glyph_sprite.dimensions = current_glyph.glyph_rect;
					glyph_sprite.dimensions.position += text_component->offset;

					glyph_sprite.uv_top_left = current_glyph.uv_rect.position;
					glyph_sprite.uv_bottom_right = glyph_sprite.uv_top_left + current_glyph.uv_rect.size;
					glyph_sprite.uv_top_right = Vadon::Utilities::Vector2(glyph_sprite.uv_bottom_right.x, glyph_sprite.uv_top_left.y);
					glyph_sprite.uv_bottom_left = Vadon::Utilities::Vector2(glyph_sprite.uv_top_left.x, glyph_sprite.uv_bottom_right.y);
					canvas_system.draw_item_sprite(base_component->canvas_item, glyph_sprite);
				}
			}
		}

		void remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
		{
			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

			Base* base_component = component_manager.get_component<Base>(entity);
			if (base_component == nullptr)
			{
				// TODO: error?
				return;
			}

			if (base_component->canvas_item.is_valid() == true)
			{
				Vadon::Render::Canvas::CanvasSystem& canvas_system = m_engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
				canvas_system.remove_item(base_component->canvas_item);
			}
		}

		void update_selectables(Vadon::ECS::World& /*ecs_world*/)
		{
#if 0
			// TODO: register callbacks, use EntityHandle to identify which selectable to use
			abcdefg;

			Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
			auto selectable_query = component_manager.run_component_query<Base&, Selectable&>();
			for (auto selectable_it = selectable_query.get_iterator(); selectable_it.is_valid() == true; selectable_it.next())
			{
				auto selectable_tuple = selectable_it.get_tuple();

				const Base& base_component = std::get<Base&>(selectable_tuple);
				const Selectable& selectable_component = std::get<Selectable&>(selectable_tuple);

				// TODO: check cursor position
				// If over selectable and clicked, activate callback
				abcdefg;
			}
#endif
		}
	};

	UI::UI(Vadon::Core::EngineCoreInterface& engine_core)
		: m_internal(std::make_unique<Internal>(engine_core))
	{
	}

	UI::~UI() = default;

	bool UI::initialize()
	{
		return m_internal->initialize();
	}

	void UI::update(Vadon::ECS::World& ecs_world)
	{
		m_internal->update(ecs_world);
	}

	void UI::update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->update_ui_element(ecs_world, entity);
	}

	void UI::remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		m_internal->remove_ui_element(ecs_world, entity);
	}
}