#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_SRV_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_SRV_HPP
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{
    using ShaderResourceViewType = Vadon::Render::ShaderResourceViewType;
    using ShaderResourceViewTypeInfo = Vadon::Render::ShaderResourceViewTypeInfo;

    using ShaderResourceViewInfo = Vadon::Render::ShaderResourceViewInfo;

    using SRVHandle = Vadon::Render::SRVHandle;

    using D3DShaderResourceView = ComPtr<ID3D11ShaderResourceView>;

    struct ShaderResourceView
    {
        ShaderResourceViewInfo info;
        D3DShaderResourceView d3d_srv;
        ResourceHandle resource;
    };
}
#endif