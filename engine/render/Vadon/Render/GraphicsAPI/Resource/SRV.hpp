#ifndef VADON_RENDER_GRAPHICSAPI_RESOURCE_SRV_HPP
#define VADON_RENDER_GRAPHICSAPI_RESOURCE_SRV_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render
{
    // NOTE: based on D3D types
    enum class ShaderResourceViewType
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
        RAW_BUFFER
    };

    struct ShaderResourceViewTypeInfo
    {
        uint32_t most_detailed_mip = 0;
        uint32_t mip_levels = 0;
        uint32_t first_array_slice = 0;
        uint32_t array_size = 0;
    };

    struct ShaderResourceViewInfo
    {
        // FIXME: these two could just be merged via std::variant, could then use variant index to determine type?
        ShaderResourceViewType type;
        GraphicsAPIDataFormat format;
        ShaderResourceViewTypeInfo type_info;
    };

    VADON_DECLARE_TYPED_POOL_HANDLE(ShaderResourceView, SRVHandle);
}
#endif