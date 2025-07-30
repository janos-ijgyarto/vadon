#include <Vadon/Private/PCH/Render.hpp>
#include <Vadon/Private/Render/Text/TextSystem.hpp>

#include <Vadon/Render/GraphicsAPI/Texture/TextureSystem.hpp>

#include <Vadon/Utilities/Data/DataUtilities.hpp>

namespace
{
	// FIXME: move to utility header!
	constexpr uint32_t round_up_power_of_two(uint32_t value)
	{
		// Source: https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
		uint32_t result = value - 1;
		result |= result >> 1;
		result |= result >> 2;
		result |= result >> 4;
		result |= result >> 8;
		result |= result >> 16;
		return (result + 1);
	}

	constexpr size_t c_glyph_pixel_metric = 64;

	constexpr auto generate_break_character_lookup()
	{
		constexpr std::array c_break_characters{ ' ', ',', '.', ';', ':', '-', '/', '\t' };
		std::array<bool, Vadon::Private::Render::TextSystem::c_font_character_count> lookup{ false };

		for (char current_char : c_break_characters)
		{
			lookup[current_char] = true;
		}

		return lookup;
	}

	constexpr auto c_break_character_lookup = generate_break_character_lookup();

	struct FTFontFaceWrapper
	{
		FT_Face ft_face;

		~FTFontFaceWrapper()
		{
			if (ft_face)
			{
				// Make sure we clean up the face when we are done
				FT_Done_Face(ft_face);
			}
		}
	};
}

namespace Vadon::Private::Render
{
	void TextSystem::FontData::create_font_atlas()
	{
		// Packing algorithm: https://observablehq.com/@mourner/simple-rectangle-packing
		// FIXME: add to engine utilities!
		struct GlyphBox
		{
			int32_t index = 0;
			Vector2i dimensions;
			Vector2i texture_position;
		};

		// Calculate total box area and maximum box width
		int32_t area = 0;
		int32_t max_width = 0;
		std::vector<GlyphBox> glyph_boxes(glyphs.size());
		{
			int32_t current_glyph_index = 0;
			auto glyph_box_it = glyph_boxes.begin();
			for (const FontGlyph& current_glyph : glyphs)
			{
				area += current_glyph.glyph_size.x * current_glyph.glyph_size.y;
				max_width = std::max(max_width, current_glyph.glyph_size.x);

				glyph_box_it->index = current_glyph_index;
				glyph_box_it->dimensions = current_glyph.glyph_size;

				++current_glyph_index;
				++glyph_box_it;
			}
		}

		// Sort the boxes for insertion by height, descending
		std::sort(glyph_boxes.begin(), glyph_boxes.end(),
			[](const GlyphBox& lhs, const GlyphBox& rhs)
			{
				return (lhs.dimensions.y > rhs.dimensions.y);
			}
		);

		max_width = std::max(static_cast<int32_t>(std::ceil(std::sqrt(area))), max_width);
		texture_dimension = round_up_power_of_two(max_width);

		struct TextureSpace
		{
			Vector2i position;
			Vector2i size;
		};

		std::vector<TextureSpace> spaces;
		{
			// Start with a single empty space, unbounded at the bottom
			TextureSpace& start_space = spaces.emplace_back();
			start_space.size.x = texture_dimension;
			start_space.size.y = std::numeric_limits<int32_t>::max();
		}

		for (const GlyphBox& current_box : glyph_boxes)
		{
			// Look through spaces backwards so that we check smaller spaces first
			for (auto space_rit = spaces.rbegin(); space_rit != spaces.rend(); ++space_rit)
			{
				TextureSpace& current_space = *space_rit;

				// Look for empty spaces that can accommodate the current box
				if ((current_box.dimensions.x > current_space.size.x) || (current_box.dimensions.y > current_space.size.y))
				{
					continue;
				}

				// Found the space; add the box to its top-left corner
				// |-------|-------|
				// |  box  |       |
				// |_______|       |
				// |         space |
				// |_______________|
				FontGlyph& current_glyph = glyphs[current_box.index];
				current_glyph.texture_offset = current_space.position;

				if ((current_box.dimensions.x == current_space.size.x) && (current_box.dimensions.y == current_space.size.y))
				{
					// Space matches the box exactly; remove it (swap & pop)
					std::swap(current_space, spaces.back());
					spaces.pop_back();

				}
				else if (current_box.dimensions.y == current_space.size.y)
				{
					// space matches the box height; update it accordingly
					// |-------|---------------|
					// |  box  | updated space |
					// |_______|_______________|
					current_space.position.x += current_box.dimensions.x;
					current_space.size.x -= current_box.dimensions.x;

				}
				else if (current_box.dimensions.x == current_space.size.x)
				{
					// space matches the box width; update it accordingly
					// |---------------|
					// |      box      |
					// |_______________|
					// | updated space |
					// |_______________|
					current_space.position.y += current_box.dimensions.y;
					current_space.size.y -= current_box.dimensions.y;
				}
				else
				{
					// otherwise the box splits the space into two spaces
					// |-------|-----------|
					// |  box  | new space |
					// |_______|___________|
					// | updated space     |
					// |___________________|
					TextureSpace new_space;
					new_space.position.x = current_space.position.x + current_box.dimensions.x;
					new_space.position.y = current_space.position.y;

					new_space.size.x = current_space.size.x - current_box.dimensions.x;
					new_space.size.y = current_box.dimensions.y;

					current_space.position.y += current_box.dimensions.y;
					current_space.size.y -= current_box.dimensions.y;

					spaces.push_back(new_space);
				}
				break;
			}
		}
	}

	std::vector<size_t> TextSystem::FontData::calculate_line_wrap_offsets(std::string_view text, const TextRenderInfo& render_info) const
	{
		std::vector<size_t> wrap_offsets;

		if (render_info.width < 0.0f)
		{
			// No wrapping, just use line ends
			size_t current_char_index = 0;
			for (char current_character : text)
			{
				if (current_character == '\n')
				{
					wrap_offsets.push_back(current_char_index);
				}
				++current_char_index;
			}
			return wrap_offsets;
		}

		float current_advance = 0.0f;

		auto line_start_it = text.begin();
		auto break_it = text.end();
		for (auto current_char_it = text.begin(); current_char_it != text.end();)
		{
			const char current_character = *current_char_it;
			if (current_character == '\n')
			{
				// Endline character, break
				wrap_offsets.push_back(std::distance(text.begin(), current_char_it));

				++current_char_it;
				line_start_it = current_char_it;

				// Reset variables
				current_advance = 0.0f;
				break_it = text.end();

				continue;
			}

			const size_t current_glyph_index = character_glyph_map[current_character];
			const FontGlyph& current_font_glyph = glyphs[current_glyph_index];

			const float current_horizontal_extent = current_advance + (current_font_glyph.draw_offset.x + current_font_glyph.glyph_size.x) * render_info.scale;
			if (current_horizontal_extent > render_info.width)
			{
				// Adding this character will exceed the line width
				// Check if we can break
				if (break_it != text.end())
				{
					// Had a break character, add break there 
					wrap_offsets.push_back(std::distance(text.begin(), break_it));

					// Restart from the character following the break
					// FIXME: could cache the advance we had from the last break position and use it
					current_char_it = break_it + 1;
					line_start_it = current_char_it;

					// Reset variables
					current_advance = 0.0f;
					break_it = text.end();
				}
				else
				{
					// No break character, have to split the word itself
					if (current_char_it == line_start_it)
					{
						// No room even for a single character, so we break right after it
						wrap_offsets.push_back(std::distance(text.begin(), current_char_it));
						++current_char_it;
					}
					else
					{
						// Break after the previous character
						wrap_offsets.push_back(std::distance(text.begin(), current_char_it - 1));
					}

					// Set the new line start
					line_start_it = current_char_it;

					// Reset variables
					current_advance = 0.0f;
					break_it = text.end();
				}
			}
			else
			{
				// Still have room, continue through the text
				if (c_break_character_lookup[current_character])
				{
					// Found a break character
					// FIXME: at the moment we demand that the break char fits as well. Should we ignore that requirement for whitespace?
					break_it = current_char_it;
				}

				// Update the advance
				current_advance += static_cast<float>(current_font_glyph.advance) * render_info.scale;
				++current_char_it;
			}
		}

		return wrap_offsets;
	}

	FontHandle TextSystem::create_font(const FontInfo& font_info, std::span<const unsigned char> font_data_buffer)
	{
		// FreeType code based in part on: https://kevinboone.me/fbtextdemo.html
		FTFontFaceWrapper font_face;
		if (FT_New_Memory_Face(m_ft_library, font_data_buffer.data(), static_cast<FT_Long>(font_data_buffer.size_bytes()), font_info.face_index, &font_face.ft_face) != 0)
		{
			return FontHandle();
		}

		if (FT_Set_Pixel_Sizes(font_face.ft_face, font_info.width, font_info.height) != 0)
		{
			return FontHandle();
		}

		FontData font_data;
		font_data.line_spacing = font_face.ft_face->height / c_glyph_pixel_metric;
		font_data.scaled_line_spacing = font_face.ft_face->size->metrics.height / c_glyph_pixel_metric;

		std::vector<Vadon::Utilities::DataRange> glyph_data_ranges;
		std::vector<float> glyph_temp_data_buffer;

		font_data.character_glyph_map.fill(0);
		font_data.glyphs.reserve(c_font_character_count);
		glyph_data_ranges.reserve(c_font_character_count);

		FT_GlyphSlot glyph_slot = font_face.ft_face->glyph;

		// First pass: gather the raw glyph data
		for (unsigned char current_char = 0; current_char < c_font_character_count; ++current_char)
		{
			// Load character glyph 
			if (FT_Get_Char_Index(font_face.ft_face, current_char) == 0)
			{
				// Character data not available
				continue;
			}

			if (FT_Load_Char(font_face.ft_face, current_char, FT_LOAD_RENDER) != 0)
			{
				// Something went wrong
				return FontHandle();
			}

			// Character has valid data, add to map
			font_data.character_glyph_map[current_char] = font_data.glyphs.size();

			// Render as SDF
			FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_SDF);

			const FT_Bitmap& glyph_bitmap = glyph_slot->bitmap;

			const uint32_t bitmap_size = glyph_bitmap.width * glyph_bitmap.rows;

			// Cache all the relevant glyph metadata
			Vadon::Utilities::DataRange& current_glyph_data_range = glyph_data_ranges.emplace_back();
			FontGlyph& current_glyph = font_data.glyphs.emplace_back();

			current_glyph_data_range.count = bitmap_size;
			current_glyph_data_range.offset = static_cast<int32_t>(glyph_temp_data_buffer.size());

			current_glyph.glyph_size = Vector2i(glyph_bitmap.width, glyph_bitmap.rows);

			const int32_t bbox_ymax = font_face.ft_face->bbox.yMax / c_glyph_pixel_metric;
			const int32_t glyph_width = font_face.ft_face->glyph->metrics.width / c_glyph_pixel_metric;
			current_glyph.advance = font_face.ft_face->glyph->metrics.horiAdvance / c_glyph_pixel_metric;

			current_glyph.draw_offset.x = (current_glyph.advance - glyph_width) / 2;
			current_glyph.draw_offset.y = bbox_ymax - font_face.ft_face->glyph->metrics.horiBearingY / c_glyph_pixel_metric;

			// Add the glyph bitmap to the buffer
			glyph_temp_data_buffer.reserve(glyph_temp_data_buffer.size() + bitmap_size);
			{
				// Convert the 8-bit values to floats
				// FIXME: use 8-bit instead?
				for (uint32_t current_row = 0; current_row < glyph_bitmap.rows; ++current_row)
				{
					const uint32_t row_offset = current_row * glyph_bitmap.pitch;
					for (uint32_t current_column = 0; current_column < glyph_bitmap.width; ++current_column)
					{
						constexpr float c_abs_signed_distance_max = 128.0f;
						constexpr float c_signed_distance_default_spread = 2.0f;

						const float signed_distance = static_cast<float>(glyph_bitmap.buffer[row_offset + current_column]) - c_abs_signed_distance_max;

						glyph_temp_data_buffer.push_back((signed_distance / c_abs_signed_distance_max) * c_signed_distance_default_spread);
					}
				}
			}
		}

		font_data.create_font_atlas();

		std::vector<float> finalized_font_texture_data(font_data.texture_dimension * font_data.texture_dimension, 0.0f);

		auto glyph_it = font_data.glyphs.begin();
		for (const Vadon::Utilities::DataRange& current_glyph_data_range : glyph_data_ranges)
		{
			const FontGlyph& current_glyph = *glyph_it;

			const int32_t texture_data_base_offset = current_glyph.texture_offset.y * font_data.texture_dimension + current_glyph.texture_offset.x;

			// Go over the data row-by-row
			for (int32_t current_row = 0; current_row < current_glyph.glyph_size.y; ++current_row)
			{
				// Copy the row into the destination
				const int32_t texture_row_offset = texture_data_base_offset + (current_row * font_data.texture_dimension);
				const int32_t glyph_row_offset = current_row * current_glyph.glyph_size.x;

				auto glyph_temp_data_begin = glyph_temp_data_buffer.begin() + current_glyph_data_range.offset + glyph_row_offset;
				auto glyph_temp_data_end = glyph_temp_data_begin + current_glyph.glyph_size.x;
				std::copy(glyph_temp_data_begin, glyph_temp_data_end, finalized_font_texture_data.begin() + texture_row_offset);
			}

			++glyph_it;
		}

		// Create font texture
		{
			Vadon::Render::TextureSystem& texture_system = m_engine_core.get_system<Vadon::Render::TextureSystem>();

			Vadon::Render::TextureInfo texture_info;
			texture_info.dimensions.x = font_data.texture_dimension;
			texture_info.dimensions.y = font_data.texture_dimension;
			texture_info.mip_levels = 1;
			texture_info.array_size = 1;
			texture_info.format = Vadon::Render::GraphicsAPIDataFormat::R32_FLOAT;
			texture_info.sample_info.count = 1;
			texture_info.usage = Vadon::Render::ResourceUsage::DEFAULT;
			texture_info.flags = Vadon::Render::TextureFlags::SHADER_RESOURCE;

			// Create texture view
			Vadon::Render::TextureSRVInfo texture_srv_info;
			texture_srv_info.format = Vadon::Render::GraphicsAPIDataFormat::R32_FLOAT;
			texture_srv_info.type = Vadon::Render::TextureSRVType::TEXTURE_2D;
			texture_srv_info.mip_levels = texture_info.mip_levels;
			texture_srv_info.most_detailed_mip = 0;

			Vadon::Render::TextureHandle font_texture_handle = texture_system.create_texture(texture_info, finalized_font_texture_data.data());
			if (font_texture_handle.is_valid() == false)
			{
				return FontHandle();
			}

			Vadon::Render::SRVHandle font_resource_view = texture_system.create_shader_resource_view(font_texture_handle, texture_srv_info);
			if (font_resource_view.is_valid() == false)
			{
				// FIXME: remove font texture?
				return FontHandle();
			}

			font_data.texture_handle = font_texture_handle;
			font_data.texture_srv = font_resource_view;
		}

		// All succesful, store the new font
		FontHandle new_font_handle = m_font_pool.add();
		FontData& new_font_data = m_font_pool.get(new_font_handle);
		new_font_data = std::move(font_data);

		return new_font_handle;
	}

	TextRenderData TextSystem::create_text_render_data(std::string_view text, const TextRenderInfo& render_info) const
	{
		TextRenderData render_data;

		const FontData& font_data = m_font_pool.get(render_info.font);
		render_data.font_texture_view = font_data.texture_srv;

		const float font_texture_dimension = static_cast<float>(font_data.texture_dimension);

		const std::vector<size_t> line_wrap_offsets = font_data.calculate_line_wrap_offsets(text, render_info);
		auto line_wrap_it = line_wrap_offsets.begin();

		// Generate glyphs corresponding to the characters in the text
		Vector2 line_offset(0, 0);
		size_t current_character_index = 0;
		for (char current_char : text)
		{
			// Generate the vertices from glyph info
			const size_t current_glyph_index = font_data.character_glyph_map[current_char];
			const FontGlyph& current_font_glyph = font_data.glyphs[current_glyph_index];

			TextGlyph& current_render_glyph = render_data.glyphs.emplace_back();

			// Get the parameters for the vertices w.r.t draw origin
			const Vector2 draw_offset = Vector2(current_font_glyph.draw_offset) * render_info.scale;

			// NOTE: font data uses window coordinates, i.e (0, 0) is top left, so we need to convert
			const Vector2 current_glyph_offset = line_offset + draw_offset;
			current_render_glyph.glyph_rect.size = Vector2(current_font_glyph.glyph_size) * render_info.scale;

			const Vector2 glyph_half_size = current_render_glyph.glyph_rect.size / 2.0f;
			current_render_glyph.glyph_rect.position = Vector2(current_glyph_offset.x + glyph_half_size.x, -(current_glyph_offset.y + glyph_half_size.y));

			// Get the location in the texture atlas
			current_render_glyph.uv_rect.position = Vector2(current_font_glyph.texture_offset) / font_texture_dimension;
			current_render_glyph.uv_rect.size = Vector2(current_font_glyph.glyph_size) / font_texture_dimension;

			if ((line_wrap_it != line_wrap_offsets.end()) && (*line_wrap_it == current_character_index))
			{
				// Reset horizontal offset, advance vertical offset
				line_offset.x = 0.0f;
				line_offset.y += font_data.scaled_line_spacing * render_info.scale;

				++line_wrap_it;
			}
			else
			{
				// Advance the horizontal offset
				line_offset.x += current_font_glyph.advance * render_info.scale;
			}

			++current_character_index;
		}

		return render_data;
	}

	TextSystem::TextSystem(Vadon::Core::EngineCoreInterface& core)
		: Vadon::Render::TextSystem(core)
	{

	}

	bool TextSystem::initialize()
	{
		if (FT_Init_FreeType(&m_ft_library) != 0)
		{
			return false;
		}

		return true;
	}
}