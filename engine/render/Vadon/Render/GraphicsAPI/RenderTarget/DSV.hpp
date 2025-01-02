#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_DSV_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_DSV_HPP
#include <Vadon/Render/GraphicsAPI/Resource/Resource.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Render
{
	enum class DepthStencilClearFlags
	{
		NONE = 0,
		DEPTH = 1 << 0,
		STENCIL = 1 << 1
	};

	struct DepthStencilClear
	{
		DepthStencilClearFlags clear_flags = DepthStencilClearFlags::NONE;
		float depth = 0.0f;
		uint8_t stencil = 0;
	};

	enum class DepthStencilViewType
	{
		UNKNOWN,
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_2D_MS,
		TEXTURE_2D_MS_ARRAY
	};

	// FIXME: use union/variant to create distinct structs!
	struct DepthStencilViewTypeInfo
	{
		uint32_t mip_slice = 0;
		// TODO: params for other view types
		// For now we only support Texture2D
	};

	enum class DepthStencilViewFlags
	{
		NONE = 0,
		READ_ONLY_DEPTH = 1 << 0,
		READ_ONLY_STENCIL = 1 << 2
	};

	struct DepthStencilViewInfo
	{
		GraphicsAPIDataFormat format = GraphicsAPIDataFormat::UNKNOWN;
		DepthStencilViewType type;
		DepthStencilViewTypeInfo type_info;
		DepthStencilViewFlags flags = DepthStencilViewFlags::NONE;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(DepthStencilView, DSVHandle);
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::DepthStencilClearFlags> : public std::true_type
	{

	};

	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::DepthStencilViewFlags> : public std::true_type
	{

	};
}
#endif