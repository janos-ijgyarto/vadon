#ifndef VADON_RENDER_RHI_RESOURCE_BUFFER_HPP
#define VADON_RENDER_RHI_RESOURCE_BUFFER_HPP
#include <Vadon/Render/RHI/Common/Format.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Render::RHI
{
	enum class BufferUsage
	{
		NONE = 0,
		SHADER_RESOURCE = 1 << 0,
		READ_WRITE = 1 << 1,
		VERTEX = 1 << 2,
		INDEX = 1 << 3,
		CONSTANT = 1 << 4,
		BYTE_ADDRESS = 1 << 5,
		STRUCTURED = 1 << 6
	};

	struct BufferDescription
	{
		BufferUsage usage = BufferUsage::NONE;
		uint64_t size = 0;
		uint32_t structure_byte_stride = 0;
		Format format = Format::UNKNOWN;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Buffer, BufferHandle);

	struct BufferOffset
	{
		BufferHandle buffer;
		uint64_t offset;
	};
}
VADON_ENABLE_BITWISE_OPERATORS(Vadon::Render::RHI::BufferUsage);
#endif