#ifndef VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURE_HPP
#define VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURE_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>

#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace Vadon::Render
{
	struct TextureSampleInfo
	{
		int count = 0;
		int quality = 0;
	};

	// FIXME: have separate info for 1D, 2D, and 3D textures?
	struct Texture2DInfo
	{
		Utilities::Vector2i dimensions = Utilities::Vector2i(0, 0);
		int mip_levels = 0;
		int array_size = 0;
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;

		TextureSampleInfo sample_info;
		BufferUsage usage = BufferUsage::DEFAULT;
		BufferBinding binding = BufferBinding::SHADER_RESOURCE;
		int access = 0;
		int misc = 0;
	};

	enum class TextureFilter
	{
		// TODO: other supported filters?
		MIN_MAG_MIP_POINT,
		MIN_MAG_MIP_LINEAR
	};

	enum class TextureAddressMode
	{
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};

	struct TextureSamplerInfo
	{
		TextureFilter filter = TextureFilter::MIN_MAG_MIP_POINT;
		TextureAddressMode address_u = TextureAddressMode::WRAP;
		TextureAddressMode address_v = TextureAddressMode::WRAP;
		TextureAddressMode address_w = TextureAddressMode::WRAP;
		float mip_lod_bias = 0.0f;
		uint32_t max_anisotropy = 0;
		GraphicsAPIComparisonFunction comparison_func = GraphicsAPIComparisonFunction::NEVER;
		float border_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float min_lod = 0.0f;
		float max_lod = 0.0f;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Texture, TextureHandle);
	VADON_DECLARE_TYPED_POOL_HANDLE(TextureSampler, TextureSamplerHandle);
}
#endif