#ifndef VADON_PRIVATE_RENDER_CANVAS_COMMANDBUFFER_HPP
#define VADON_PRIVATE_RENDER_CANVAS_COMMANDBUFFER_HPP
#include <Vadon/Render/Canvas/CommandBuffer.hpp>

namespace Vadon::Private::Render::Canvas
{
	enum class CommandType : uint32_t
	{
		TRIANGLE,
		RECTANGLE,
		CIRCLE,
		SPRITE,
		BATCH,
		SET_TEXTURE,
		SET_MATERIAL,
		SET_RENDER_STATE
	};

	struct CommandInfo
	{
		static constexpr uint32_t c_type_mask = (1 << 16) - 1;

		uint32_t type; // from CommandType enum
		uint32_t flags; // Flags used by certain command types

		uint32_t to_uint() const
		{
			return (type & c_type_mask) | (flags << 16);
		}
	};

	using CommandBuffer = Vadon::Render::Canvas::CommandBuffer;
}
#endif