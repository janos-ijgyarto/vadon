#ifndef VADON_RENDER_RHI_RESOURCE_TEXTURE_HPP
#define VADON_RENDER_RHI_RESOURCE_TEXTURE_HPP
#include <Vadon/Render/RHI/Common/Format.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Render::RHI
{
	enum class TextureType
	{
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D
	};

	enum class TextureUsage
	{
		NONE = 0,
		SHADER_RESOURCE = 1 << 0,
		READ_WRITE = 1 << 1,
		RENDER_TARGET = 1 << 2,
		DEPTH_STENCIL = 1 << 3
	};

	struct TextureDescription
	{
		TextureType type = TextureType::TEXTURE_2D;
		Format format = Format::UNKNOWN;
		TextureUsage usage = TextureUsage::NONE;

		uint16_t width = 1;
		uint16_t height = 1;
		uint16_t depth = 1;
		uint16_t mip_count = 1;
		uint16_t layer_count = 1;		
		uint8_t sample_count = 1;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Texture, TextureHandle);
}
VADON_ENABLE_BITWISE_OPERATORS(Vadon::Render::RHI::TextureUsage);
#endif