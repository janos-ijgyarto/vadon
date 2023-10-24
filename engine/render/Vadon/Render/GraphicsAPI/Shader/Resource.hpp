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
        BUFFEREX
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

    // NOTE: based on D3D bind flags
    enum class ResourceBindFlags
    {
        NONE = 0,
        VERTEX_BUFFER = 1 << 0,
        INDEX_BUFFER = 1 << 1,
        CONSTANT_BUFFER = 1 << 2,
        SHADER_RESOURCE = 1 << 3,
        STREAM_OUTPUT = 1 << 4,
        RENDER_TARGET = 1 << 5,
        DEPTH_STENCIL = 1 << 6,
        UNORDERED_ACCESS = 1 << 7,
        DECODER = 1 << 9,
        VIDEO_ENCODER = 1 << 10
    };

    // NOTE: based on D3D resource misc. flags
    enum class ResourceMiscFlags
    {
        NONE = 0,
        GENERATE_MIPS = 1 << 0,
        SHARED = 1 << 1,
        TEXTURECUBE = 1 << 2,
        DRAWINDIRECT_ARGS = 1 << 4,
        BUFFER_ALLOW_RAW_VIEWS = 1 << 5,
        BUFFER_STRUCTURED = 1 << 6,
        RESOURCE_CLAMP = 1 << 7,
        SHARED_KEYEDMUTEX = 1 << 8,
        GDI_COMPATIBLE = 1 << 9,
        SHARED_NTHANDLE = 1 << 11,
        RESTRICTED_CONTENT = 1 << 12,
        RESTRICT_SHARED_RESOURCE = 1 << 13,
        RESTRICT_SHARED_RESOURCE_DRIVER = 1 << 14,
        GUARDED = 1 << 15,
        TILE_POOL = 1 << 17,
        TILED = 1 << 18,
        HW_PROTECTED = 1 << 19,
        SHARED_DISPLAYABLE = 1 << 20,
        SHARED_EXCLUSIVE_WRITER = 1 << 21
    };

    // NOTE: based on D3D CPU access flags
    enum class ResourceAccessFlags
    {
        NONE = 0,
        WRITE = 1 << 16,
        READ = 1 << 17
    };

    // FIXME: have a struct for each resource type?
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
    struct EnableEnumBitwiseOperators<Vadon::Render::ResourceBindFlags> : public std::true_type
    {

    };

    template<>
    struct EnableEnumBitwiseOperators<Vadon::Render::ResourceMiscFlags> : public std::true_type
    {

    };

    template<>
    struct EnableEnumBitwiseOperators<Vadon::Render::ResourceAccessFlags> : public std::true_type
    {

    };
}
#endif