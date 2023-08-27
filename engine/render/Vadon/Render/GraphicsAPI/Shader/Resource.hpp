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

    // FIXME: have a struct for each resource type?
    struct ShaderResourceTypeData
    {
        int32_t most_detailed_mip = 0;
        int32_t mip_levels = 0;
        int32_t first_array_slice = 0;
        int32_t array_size = 0;
    };

    struct ShaderResourceViewInfo
    {
        GraphicsAPIDataFormat format;
        ShaderResourceType type;

        ShaderResourceTypeData resource_type_data;
    };

    VADON_DECLARE_TYPED_POOL_HANDLE(ShaderResourceView, ShaderResourceViewHandle);
}
#endif