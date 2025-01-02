#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_UAV_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_RESOURCE_UAV_HPP
#include <Vadon/Render/GraphicsAPI/Resource/UAV.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Resource/Resource.hpp>
namespace Vadon::Private::Render::DirectX
{    
    using UnorderedAccessViewType = Vadon::Render::UnorderedAccessViewType;
    using UnorderedAccessViewTypeInfo = Vadon::Render::UnorderedAccessViewTypeInfo;
    using UnorderedAccessViewInfo = Vadon::Render::UnorderedAccessViewInfo;
    using UAVHandle = Vadon::Render::UAVHandle;

    using D3DUnorderedAccessView = ComPtr<ID3D11UnorderedAccessView>;

    struct UnorderedAccessView
    {
        UnorderedAccessViewInfo info;
        D3DUnorderedAccessView d3d_uav;
        ResourceHandle resource;
    };
}
#endif