#ifndef VADON_PRIVATE_RENDER_TEXT_TEXTSYSTEM_HPP
#define VADON_PRIVATE_RENDER_TEXT_TEXTSYSTEM_HPP
#include <Vadon/Render/Text/TextSystem.hpp>
#include <Vadon/Private/Render/Text/Font.hpp>

#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Private/Render/Utilities/Vector.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <freetype/freetype.h>
#include <array>

namespace Vadon::Private::Render
{
	class TextSystem : public Vadon::Render::TextSystem
	{
	public:
		static constexpr size_t c_font_character_count = 128; // FIXME: make this user-configurable?

		FontHandle create_font(const FontInfo& font_info, std::span<const unsigned char> font_data_buffer) override;
		TextRenderData create_text_render_data(std::string_view text, const TextRenderInfo& render_info) const override;
	protected:
		struct FontGlyph
		{
			Vector2i texture_offset;
			Vector2i glyph_size;

			int32_t advance = 0;
			Vector2i draw_offset;
		};

		struct FontData
		{
			FontInfo font_info;

			int32_t line_spacing = 0;
			int32_t scaled_line_spacing = 0;

			std::array<size_t, c_font_character_count> character_glyph_map;
			std::vector<FontGlyph> glyphs;
			int32_t texture_dimension = 0;

			Vadon::Render::TextureHandle texture_handle;
			Vadon::Render::SRVHandle texture_srv;

			void create_font_atlas();
			std::vector<size_t> calculate_line_wrap_offsets(std::string_view text, const TextRenderInfo& render_info) const;
		};

		TextSystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize();
		void shutdown();

		FT_Library m_ft_library;

		Vadon::Utilities::ObjectPool<Vadon::Render::Font, FontData> m_font_pool;

		friend class RenderSystem;
	};
}
#endif