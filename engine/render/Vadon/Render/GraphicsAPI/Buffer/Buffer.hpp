#ifndef VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#define VADON_RENDER_GRAPHICSAPI_BUFFER_BUFFER_HPP
#include <Vadon/Render/GraphicsAPI/Resource/Resource.hpp>
namespace Vadon::Render
{
	enum class BufferType
	{
		VERTEX,
		INDEX,
		CONSTANT,
		RAW, // i.e ByteAddressBuffer
		TYPED, // e.g Buffer<float4>
		STRUCTURED
	};

	enum class BufferFlags
	{
		NONE = 0,
		SHADER_RESOURCE = 1 << 0,
		UNORDERED_ACCESS = 1 << 1,
		RENDER_TARGET = 1 << 2
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

	VADON_GRAPHICSAPI_DECLARE_TYPED_RESOURCE_HANDLE(class Buffer, BufferHandle);

	enum class BufferSRVType
	{
		BUFFER,
		RAW_BUFFER
	};

	enum class BufferSRVFlags
	{
		NONE = 0,
		RAW = 1 << 0
	};

	struct BufferSRVInfo
	{
		BufferSRVType type = BufferSRVType::BUFFER;
		GraphicsAPIDataFormat format;
		uint32_t first_element;
		uint32_t num_elements;
		BufferSRVFlags flags = BufferSRVFlags::NONE;
	};

	enum class BufferUAVFlags
	{
		NONE = 0,
		RAW = 1 << 0,
		APPEND = 1 << 1,
		COUNTER = 1 << 2
	};

	// TODO: UAV info?
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::BufferFlags> : public std::true_type
	{

	};

	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::BufferUAVFlags> : public std::true_type
	{

	};
}
#endif