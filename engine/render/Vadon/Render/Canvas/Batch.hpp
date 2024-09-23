#ifndef VADON_RENDER_CANVAS_BATCH_HPP
#define VADON_RENDER_CANVAS_BATCH_HPP
#include <Vadon/Render/Canvas/Transform.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
namespace Vadon::Render::Canvas
{
	// TODO: any other info for batches?
	VADON_DECLARE_TYPED_POOL_HANDLE(Batch, BatchHandle);

	struct BatchDrawCommand
	{
		BatchHandle batch;
		Transform transform;
		Vadon::Utilities::DataRange range;
	};
}
#endif