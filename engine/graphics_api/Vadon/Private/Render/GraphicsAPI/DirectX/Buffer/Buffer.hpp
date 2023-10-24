#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFER_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFER_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Shader/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using BufferInfo = Vadon::Render::BufferInfo;

	using D3DBuffer = ComPtr<ID3D11Buffer>;

	struct Buffer
	{
		Buffer(const BufferInfo& buffer_info) : info(buffer_info) {}

		BufferInfo info;

		D3DBuffer d3d_buffer;
	};

	using BufferHandle = Vadon::Render::BufferHandle;
}
#endif