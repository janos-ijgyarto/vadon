#ifndef VADON_RENDER_GRAPHICSAPI_RESOURCE_UAV_HPP
#define VADON_RENDER_GRAPHICSAPI_RESOURCE_UAV_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render
{
    enum class UnorderedAccessViewType
    {
        UNKNOWN,
        BUFFER,
        TEXTURE_1D,
        TEXTURE_1D_ARRAY,
        TEXTURE_2D,
        TEXTURE_2D_ARRAY,
        TEXTURE_3D
    };

    struct UnorderedAccessViewTypeInfo
    {
        // FIXME: for buffers, this is FirstElement, NumElements, and Flags
        // Use union or std::variant to disambiguate!
        uint32_t mip_slice = 0;
        uint32_t first_array_slice = 0;
        uint32_t array_size = 0;
    };

    struct UnorderedAccessViewInfo
    {
        GraphicsAPIDataFormat format;
        UnorderedAccessViewType type;
        UnorderedAccessViewTypeInfo type_info;
    };

    VADON_DECLARE_TYPED_POOL_HANDLE(UnorderedAccessView, UAVHandle);
}
#endif