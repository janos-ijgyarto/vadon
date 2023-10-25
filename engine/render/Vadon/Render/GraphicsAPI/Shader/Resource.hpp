#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_RESOURCE_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Render
{
    // NOTE: based on D3D types
    enum class ResourceType
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

    // NOTE: based on D3D usage
    enum class ResourceUsage
    {
        DEFAULT,
        IMMUTABLE,
        DYNAMIC,
        STAGING,
        USAGE_COUNT
    };

    enum class ResourceCPUAccessFlags
    {
        NONE = 0,
        READ = 1 << 0,
        WRITE = 1 << 1
    };

    // FIXME: have a different struct per resource type, e.g via std::variant?
    struct ResourceTypeInfo
    {
        int32_t most_detailed_mip = 0;
        int32_t mip_levels = 0;
        int32_t first_array_slice = 0;
        int32_t array_size = 0;
    };

    struct ResourceViewInfo
    {
        GraphicsAPIDataFormat format;
        ResourceType type;
        ResourceTypeInfo type_info;
    };

    VADON_DECLARE_TYPED_POOL_HANDLE(ResourceView, ResourceViewHandle);
}
namespace Vadon::Utilities
{
    template<>
    struct EnableEnumBitwiseOperators<Vadon::Render::ResourceCPUAccessFlags> : public std::true_type
    {

    };
}
#endif