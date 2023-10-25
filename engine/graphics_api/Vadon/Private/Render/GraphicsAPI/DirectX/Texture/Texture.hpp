#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_TEXTURE_TEXTURE_HPP
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using TextureFlags = Vadon::Render::TextureFlags;
	using TextureSampleInfo = Vadon::Render::TextureSampleInfo;
	using TextureInfo = Vadon::Render::TextureInfo;

	using TextureHandle = Vadon::Render::TextureHandle;

	using D3DTexture1D = ComPtr<ID3D11Texture1D>;
	using D3DTexture2D = ComPtr<ID3D11Texture2D>;
	using D3DTexture3D = ComPtr<ID3D11Texture3D>;

	struct Texture
	{
		TextureInfo info;
		D3DResource d3d_texture_resource;
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

	using TextureResourceViewType = Vadon::Render::TextureResourceViewType;
	using TextureResourceViewInfo = Vadon::Render::TextureResourceViewInfo;
}
#endif