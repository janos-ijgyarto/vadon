#ifndef VADON_RENDER_TEXT_FONT_HPP
#define VADON_RENDER_TEXT_FONT_HPP
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
namespace Vadon::Render
{
	struct FontInfo
	{
		// FIXME: select the range of characters to load?
		std::string name;
		int32_t face_index = 0;
		int32_t width = 0;
		int32_t height = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Font, FontHandle);

	struct TextRenderInfo
	{
		FontHandle font;
		float width = -1.0f;
		float scale = 1.0f;
	};

	struct TextGlyph
	{
		Rectangle glyph_rect;
		Rectangle uv_rect;
	};

	using TextGlyphList = std::vector<TextGlyph>;

	struct TextRenderData
	{
		SRVHandle font_texture_view;
		TextGlyphList glyphs;

		void clear() { glyphs.clear(); font_texture_view.invalidate(); }
		bool is_valid() const { return font_texture_view.is_valid() && (!glyphs.empty()); }
	};
}
#endif