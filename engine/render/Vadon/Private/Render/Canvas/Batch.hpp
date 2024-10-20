#ifndef VADON_PRIVATE_RENDER_CANVAS_BATCH_HPP
#define VADON_PRIVATE_RENDER_CANVAS_BATCH_HPP
#include <Vadon/Render/Canvas/Batch.hpp>
#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>
namespace Vadon::Private::Render::Canvas
{
	using BatchHandle = Vadon::Render::Canvas::BatchHandle;
	using BatchDrawCommand = Vadon::Render::Canvas::BatchDrawCommand;

	enum class BatchCommandType : unsigned char
	{
		TRIANGLE,
		RECTANGLE,
		SPRITE
	};

	using BatchCommandBuffer = Vadon::Utilities::PacketQueue;

	struct BatchData
	{
		// TODO: anything else?
		BatchCommandBuffer command_buffer;
	};
}
#endif