#ifndef VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#define VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#include <Vadon/Render/Canvas/Item.hpp>
#include <Vadon/Private/Render/Canvas/CommandBuffer.hpp>
#include <Vadon/Private/Render/Canvas/Material.hpp>
namespace Vadon::Private::Render::Canvas
{
	using ItemInfo = Vadon::Render::Canvas::ItemInfo;
	using ItemHandle = Vadon::Render::Canvas::ItemHandle;

	struct ItemData
	{
		ItemInfo info;
		MaterialOverride material_override;
		bool visible = true;
		CommandBuffer command_buffer;
	};
}
#endif