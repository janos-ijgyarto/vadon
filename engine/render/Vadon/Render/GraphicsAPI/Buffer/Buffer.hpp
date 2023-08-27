#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
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

	enum class BufferBindFlags
	{
		NONE = 0,
		VERTEX = 1 << 0,
		INDEX = 1 << 1,
		CONSTANT = 1 << 2,
		SHADER_RESOURCE = 1 << 3
	};

	// TODO: other flags, e.g structured buffer

	struct BufferInfo
	{
		BufferUsage usage;
		BufferBindFlags bind_flags;
		int element_size = 0;
		int capacity = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Buffer, BufferHandle);
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::BufferBindFlags> : public std::true_type
	{

	};
}
#endif