#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
namespace Vadon::Render
{
	// FIXME: for buffers, we probably should constrain the info contents to data that is relevant for buffers
	// The usage/bind/etc. are generic, applicable to different resources in different ways
	// Could have enums specific to buffers, then combine with utility structs/functions that convert to the relevant API data
	struct BufferInfo
	{
		ResourceUsage usage = ResourceUsage::DEFAULT;
		ResourceBindFlags bind_flags = ResourceBindFlags::NONE;
		ResourceMiscFlags misc_flags = ResourceMiscFlags::NONE;
		ResourceAccessFlags access_flags = ResourceAccessFlags::NONE;
		int32_t element_size = 0;
		int32_t capacity = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Buffer, BufferHandle);
}
#endif