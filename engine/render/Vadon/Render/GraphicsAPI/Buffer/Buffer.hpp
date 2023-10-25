#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
namespace Vadon::Render
{
	enum class BufferType
	{
		VERTEX,
		INDEX,
		CONSTANT,
		RAW, // i.e ByteAddressBuffer
		RESOURCE, // "Regular" buffer, e.g Buffer<float4>
		STRUCTURED
	};

	enum class BufferFlags
	{
		NONE = 0,
		RESOURCE_VIEW = 1 << 0,
		UNORDERED_ACCESS_VIEW = 1 << 1
	};

	struct BufferInfo
	{
		BufferType type = BufferType::VERTEX;
		ResourceUsage usage = ResourceUsage::DEFAULT;
		BufferFlags flags = BufferFlags::NONE;
		ResourceCPUAccessFlags access_flags = ResourceCPUAccessFlags::NONE;
		int32_t element_size = 0;
		int32_t capacity = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Buffer, BufferHandle);

	struct BufferResourceViewInfo
	{
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;
		int32_t first_element = 0;
		int32_t num_elements = 0;
	};
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::BufferFlags> : public std::true_type
	{

	};
}
#endif