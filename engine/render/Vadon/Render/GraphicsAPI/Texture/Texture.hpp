#ifndef VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURE_HPP
#define VADON_RENDER_GRAPHICSAPI_TEXTURE_TEXTURE_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
#include <Vadon/Utilities/Math/Vector.hpp>
namespace Vadon::Render
{
	enum class TextureFlags
	{
		NONE = 0,
		RESOURCE_VIEW = 1 << 0,
		UNORDERED_ACCESS_VIEW = 1 << 1,
		RENDER_TARGET = 1 << 2,
		DEPTH_STENCIL = 1 << 3,
		CUBE = 1 << 4
	};

	struct TextureSampleInfo
	{
		int32_t count = 0;
		int32_t quality = 0;
	};

	struct TextureInfo
	{
		Utilities::Vector3i dimensions = Utilities::Vector3i(0, 0, 0);
		int32_t mip_levels = 0;
		int32_t array_size = 0;
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;

		TextureSampleInfo sample_info;
		ResourceUsage usage = ResourceUsage::DEFAULT;
		TextureFlags flags = TextureFlags::NONE;
		ResourceCPUAccessFlags access_flags = ResourceCPUAccessFlags::NONE;

		bool is_valid() const { return (dimensions.x > 0); }
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

	enum class TextureResourceViewType
	{
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_2D_MS,
		TEXTURE_2D_MS_ARRAY,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_CUBE_ARRAY
	};

	// FIXME: have a different struct per texture type, e.g via std::variant?
	struct TextureResourceViewInfo
	{
		TextureResourceViewType type = TextureResourceViewType::TEXTURE_1D;
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;

		int32_t most_detailed_mip = 0;
		int32_t mip_levels = 0;
		int32_t first_array_slice = 0;
		int32_t array_size = 0;
	};


	struct TextureObject
	{
		TextureHandle texture;
		ResourceViewHandle resource_view;
	};
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::TextureFlags> : public std::true_type
	{

	};
}
#endif