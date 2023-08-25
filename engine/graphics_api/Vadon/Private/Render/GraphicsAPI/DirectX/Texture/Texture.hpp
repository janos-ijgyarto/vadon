#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using TextureSampleInfo = Vadon::Render::TextureSampleInfo;
	using Texture2DInfo = Vadon::Render::Texture2DInfo;

	using TextureHandle = Vadon::Render::TextureHandle;

	using D3DTexture2D = ComPtr<ID3D11Texture2D>;

	struct Texture
	{
		// FIXME: store different texture types, e.g as variant?
		Texture2DInfo info;
		D3DTexture2D d3d_texture_2d;

		ShaderResourceHandle resource;
	};

	using TextureSamplerInfo = Vadon::Render::TextureSamplerInfo;
	using TextureFilter = Vadon::Render::TextureFilter;
	using TextureAddressMode = Vadon::Render::TextureAddressMode;
	using TextureSamplerHandle = Vadon::Render::TextureSamplerHandle;

	using D3DSamplerState = ComPtr<ID3D11SamplerState>;

	struct TextureSampler
	{
		TextureSamplerInfo info;
		D3DSamplerState sampler_state;
	};
}
#endif