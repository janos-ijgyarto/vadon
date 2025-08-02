#ifndef VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#define VADON_RENDER_GRAPHICSAPI_RENDERTARGET_RENDERTARGET_HPP
#include <Vadon/Math/Rectangle.hpp>
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Render/GraphicsAPI/Resource/Resource.hpp>
namespace Vadon::Render
{
	// Maps vertex positions (in clip space) into render target positions (i.e decides where the result of rendering ends up in the actual target)
	struct Viewport
	{
		Vadon::Math::Rectangle dimensions = { { 0, 0 }, { 0, 0 } };
		Vadon::Math::Vector2 depth_min_max = Vadon::Math::Vector2(0.0f, 1.0f);
	};

	// NOTE: based on D3D types
	enum class RenderTargetViewType
	{
		UNKNOWN,
		BUFFER,
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_2D_MS,
		TEXTURE_2D_MS_ARRAY,
		TEXTURE_3D
	};

	// FIXME: use union or variant to have distinct structs for each type!
	// For now, only support Tex2D RTV
	struct RenderTargetViewTypeInfo
	{
		uint32_t mip_slice = 0;
	};

	struct RenderTargetViewInfo
	{
		GraphicsAPIDataFormat format;
		RenderTargetViewType type;
		RenderTargetViewTypeInfo type_info;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(RenderTargetView, RTVHandle);
}
#endif