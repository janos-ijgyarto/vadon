#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_RESOURCE_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render
{
    enum class ShaderResourceType
    {
        UNKNOWN,
        BUFFER,
        TEXTURE_1D,
        TEXTURE_1D_ARRAY,
        TEXTURE_2D,
        TEXTURE_2D_ARRAY,
        TEXTURE_2D_MS,
        TEXTURE_2D_MS_ARRAY,
        TEXTURE_3D,
        TEXTURE_CUBE,
        TEXTURE_CUBE_ARRAY,
        BUFFEREX
    };

    struct Texture2DSRVInfo
    {
        int32_t most_detailed_mip;
        int32_t mip_levels;
    };

    struct ShaderResourceInfo
    {
        GraphicsAPIDataFormat format;
        ShaderResourceType type;

        // TODO: use a variant for the type-specific data? Or use separate structs?
        Texture2DSRVInfo texture_info;
    };

    VADON_DECLARE_TYPED_POOL_HANDLE(ShaderResource, ShaderResourceHandle);
}
#endif