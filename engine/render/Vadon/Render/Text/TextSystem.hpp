#ifndef VADON_RENDER_TEXT_TEXTSYSTEM_HPP
#define VADON_RENDER_TEXT_TEXTSYSTEM_HPP
#include <Vadon/Render/RenderModule.hpp>
#include <Vadon/Render/Text/Font.hpp>

#include <span>

namespace Vadon::Render
{
	class TextSystem : public RenderSystem<TextSystem>
	{
	public:
		virtual FontHandle create_font(const FontInfo& font_info, std::span<unsigned char> font_data_buffer) = 0;
		virtual TextRenderData create_text_render_data(std::string_view text, const TextRenderInfo& render_info) const = 0;
	protected:
		TextSystem(Core::EngineCoreInterface& core)
			: System(core)
		{
		}
	};
}
#endif