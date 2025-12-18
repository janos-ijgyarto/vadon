#ifndef VADON_PRIVATE_RENDER_CANVAS_BATCH_HPP
#define VADON_PRIVATE_RENDER_CANVAS_BATCH_HPP
#include <Vadon/Render/Canvas/Batch.hpp>

#include <Vadon/Private/Render/Canvas/CommandBuffer.hpp>
namespace Vadon::Private::Render::Canvas
{
	using BatchHandle = Vadon::Render::Canvas::BatchHandle;

	using BatchDrawCommand = Vadon::Render::Canvas::BatchDrawCommand;

	struct BatchData
	{
		// TODO: anything else?
		CommandBuffer command_buffer;
	};
}
#endif