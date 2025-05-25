#include <VadonDemo/UI/UI.hpp>

#include <VadonDemo/Core/Core.hpp>

#include <VadonDemo/UI/Component.hpp>
#include <VadonDemo/UI/DefaultFont.inl>

#include <VadonDemo/Render/Component.hpp>

#include <Vadon/ECS/World/World.hpp>

#include <Vadon/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Render/Text/TextSystem.hpp>

namespace VadonDemo::UI
{
	void UI::register_types()
	{
		Base::register_component();
		Frame::register_component();
		Text::register_component();
		Selectable::register_component();
	}

	void UI::register_selectable_callback(std::string_view key, SelectableCallback callback)
	{
		m_selectable_callbacks[std::string(key)].push_back(callback);
	}

	void UI::update(Vadon::ECS::World& ecs_world, const CursorState& cursor)
	{
		update_selectables(ecs_world, cursor);
	}

	void UI::update_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto component_tuple = component_manager.get_component_tuple<Base, Frame, Text, Selectable, Render::CanvasComponent>(entity);
		Base* base_component = std::get<Base*>(component_tuple);
		if (base_component == nullptr)
		{
			// TODO: error?
			return;
		}

		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
		if (canvas_component == nullptr)
		{
			// Cannot draw without canvas component
			return;
		}

		if (canvas_component->canvas_item.is_valid() == false)
		{
			// Canvas item not yet initialized
			return;
		}

		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
		canvas_system.clear_item(canvas_component->canvas_item);

		// Update transform
		{
			Vadon::Render::Canvas::Transform transform;
			transform.position = base_component->position;

			canvas_system.set_item_transform(canvas_component->canvas_item, transform);
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

			canvas_system.draw_item_rectangle(canvas_component->canvas_item, frame_rect);
		}

		const Text* text_component = std::get<Text*>(component_tuple);
		if ((text_component != nullptr) && (text_component->text.empty() == false))
		{
			Vadon::Render::TextSystem& text_system = engine_core.get_system<Vadon::Render::TextSystem>();

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

				glyph_sprite.uv_dimensions.position = current_glyph.uv_rect.position;
				glyph_sprite.uv_dimensions.size = current_glyph.uv_rect.size;
				canvas_system.draw_item_sprite(canvas_component->canvas_item, glyph_sprite);
			}
		}
	}

	void UI::remove_ui_element(Vadon::ECS::World& ecs_world, Vadon::ECS::EntityHandle entity)
	{
		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();

		auto component_tuple = component_manager.get_component_tuple<Base, Render::CanvasComponent>(entity);
		Base* base_component = std::get<Base*>(component_tuple);
		if (base_component == nullptr)
		{
			// TODO: error?
			return;
		}

		Render::CanvasComponent* canvas_component = std::get<Render::CanvasComponent*>(component_tuple);
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
		canvas_system.clear_item(canvas_component->canvas_item);
	}

	UI::UI(VadonDemo::Core::Core& core)
		: m_core(core)
	{
	}
	
	bool UI::initialize()
	{
		if (load_default_font() == false)
		{
			return false;
		}

		{
			Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
			Vadon::Render::Canvas::CanvasSystem& canvas_system = engine_core.get_system<Vadon::Render::Canvas::CanvasSystem>();
			m_text_sdf_material = canvas_system.create_material(Vadon::Render::Canvas::MaterialInfo{ .name = "default_text_sdf" });
			if (m_text_sdf_material.is_valid() == false)
			{
				return false;
			}
			canvas_system.set_material_sdf(m_text_sdf_material, Vadon::Render::Canvas::SDFParameters{ .flags = Vadon::Render::Canvas::SDFParameters::Flags::ENABLED });
		}

		return true;
	}

	void UI::global_config_updated()
	{
		// TODO: anything?
	}

	// TODO: manage fonts via resources!
	bool UI::load_default_font()
	{
		Vadon::Core::EngineCoreInterface& engine_core = m_core.get_engine_core();
		Vadon::Render::TextSystem& text_system = engine_core.get_system<Vadon::Render::TextSystem>();

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
	
	void UI::update_selectables(Vadon::ECS::World& ecs_world, const CursorState& cursor)
	{
		if (cursor.clicked == false)
		{
			return;
		}

		Vadon::ECS::ComponentManager& component_manager = ecs_world.get_component_manager();
		auto selectable_query = component_manager.run_component_query<Base&, Selectable&>();
		for (auto selectable_it = selectable_query.get_iterator(); selectable_it.is_valid() == true; selectable_it.next())
		{
			auto selectable_tuple = selectable_it.get_tuple();

			const Base& base_component = std::get<Base&>(selectable_tuple);
			const Selectable& selectable_component = std::get<Selectable&>(selectable_tuple);

			if (base_component.enabled == false)
			{
				continue;
			}

			if (selectable_component.clicked_key.empty() == true)
			{
				continue;
			}

			const Vadon::Utilities::Vector2 min_corner(base_component.position.x, base_component.position.y - base_component.dimensions.y);
			const Vadon::Utilities::Vector2 max_corner(base_component.position.x + base_component.dimensions.x, base_component.position.y);

			if (Vadon::Utilities::any(Vadon::Utilities::lessThan(cursor.position, min_corner))
				|| Vadon::Utilities::any(Vadon::Utilities::greaterThan(cursor.position, max_corner)))
			{
				continue;
			}

			// TODO: sort by Z order and pick the top
			// For now just activate all
			signal_selectable_callbacks(selectable_component.clicked_key);
		}
	}

	void UI::signal_selectable_callbacks(std::string_view key)
	{
		auto selectable_it = m_selectable_callbacks.find(std::string(key));
		if (selectable_it == m_selectable_callbacks.end())
		{
			return;
		}

		for (const SelectableCallback& current_callback : selectable_it->second)
		{
			current_callback(key);
		}
	}
}