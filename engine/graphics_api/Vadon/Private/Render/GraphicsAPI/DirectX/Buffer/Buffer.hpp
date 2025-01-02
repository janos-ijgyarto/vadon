#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFER_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_BUFFER_BUFFER_HPP
#include <Vadon/Render/GraphicsAPI/Buffer/Buffer.hpp>

#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
	using BufferType = Vadon::Render::BufferType;
	using BufferFlags = Vadon::Render::BufferFlags;
	using BufferInfo = Vadon::Render::BufferInfo;

	using D3DBuffer = ComPtr<ID3D11Buffer>;

	struct Buffer : public Resource
	{
		BufferInfo info;
		D3DBuffer d3d_buffer;
	};

	VADON_GRAPHICSAPI_DECLARE_TYPED_RESOURCE_HANDLE(Buffer, D3DBufferHandle);

	using BufferHandle = Vadon::Render::BufferHandle;

	using BufferSRVType = Vadon::Render::BufferSRVType;
	using BufferSRVFlags = Vadon::Render::BufferSRVFlags;
	using BufferSRVInfo = Vadon::Render::BufferSRVInfo;

	using BufferUAVFlags = Vadon::Render::BufferUAVFlags;
}
#endif