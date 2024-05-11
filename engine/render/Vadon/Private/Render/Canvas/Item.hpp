#ifndef VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#define VADON_PRIVATE_RENDER_CANVAS_ITEM_HPP
#include <Vadon/Render/Canvas/Item.hpp>
namespace Vadon::Private::Render::Canvas
{
	using ItemInfo = Vadon::Render::Canvas::ItemInfo;
	using ItemHandle = Vadon::Render::Canvas::ItemHandle;

	enum class CommandType : unsigned char
	{
		TRIANGLE,
		RECTANGLE,
		SPRITE
	};

	struct CommandBuffer
	{
		std::vector<std::byte> data;

		template<typename T>
		void add_primitive(CommandType type, const T& primitive_data)
		{
			const std::byte* type_ptr = (const std::byte*)&type;
			const std::byte* data_ptr = (const std::byte*)&primitive_data;
			data.insert(data.end(), type_ptr, type_ptr + sizeof(CommandType));
			data.insert(data.end(), data_ptr, data_ptr + sizeof(T));
		}

		void clear()
		{
			data.clear();
		}
	};

	struct ItemData
	{
		ItemInfo info;
		CommandBuffer command_buffer;
	};
}
#endif