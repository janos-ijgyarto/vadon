#ifndef VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#define VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Private/Render/Canvas/Batch.hpp>
namespace Vadon::Private::Render::Canvas
{
	using ItemInfo = Vadon::Render::Canvas::ItemInfo;
	using ItemHandle = Vadon::Render::Canvas::ItemHandle;

	enum class ItemCommandType
	{
		DRAW_BATCH,
		DRAW_DIRECT
	};

	using ItemCommandBuffer = Vadon::Utilities::PacketQueue;

	template<typename T, BatchCommandType BATCH_COMMAND>
	struct ItemDirectDrawCommand
	{
		ItemDirectDrawCommand(const T& data = T{})
			: batch_type(Vadon::Utilities::to_integral(BATCH_COMMAND))
			, command_data(data)
		{}

		uint32_t batch_type;
		T command_data;
	};

	struct ItemData
	{
		ItemInfo info;
		ItemCommandBuffer command_buffer;
	};
}
#endif