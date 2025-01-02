#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using TextureFlags = Vadon::Render::TextureFlags;
	using TextureSampleInfo = Vadon::Render::TextureSampleInfo;
	using TextureInfo = Vadon::Render::TextureInfo;

	using TextureHandle = Vadon::Render::TextureHandle;

	using D3DTexture1D = ComPtr<ID3D11Texture1D>;
	using D3DTexture2D = ComPtr<ID3D11Texture2D>;
	using D3DTexture3D = ComPtr<ID3D11Texture3D>;

	struct Texture : public Resource
	{
		TextureInfo info;

		// FIXME: more elegant way to do this?
		D3DTexture1D d3d_texture_1d;
		D3DTexture2D d3d_texture_2d;
		D3DTexture3D d3d_texture_3d;
	};

	VADON_GRAPHICSAPI_DECLARE_TYPED_RESOURCE_HANDLE(Texture, D3DTextureHandle);

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

	using TextureSRVType = Vadon::Render::TextureSRVType;
	using TextureSRVInfo = Vadon::Render::TextureSRVInfo;
}
#endif