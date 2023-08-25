#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render
{
	enum class BufferUsage
	{
		DEFAULT,
		IMMUTABLE,
		DYNAMIC,
		STAGING,
		USAGE_COUNT
	};

	enum class BufferBinding
	{
		VERTEX,
		INDEX,
		CONSTANT,
		SHADER_RESOURCE,
		BINDING_COUNT
	};

	// TODO: other flags, e.g structured buffer

	struct BufferInfo
	{
		BufferUsage usage;
		BufferBinding binding;
		int element_size = 0;
		int capacity = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Buffer, BufferHandle);
}
#endif