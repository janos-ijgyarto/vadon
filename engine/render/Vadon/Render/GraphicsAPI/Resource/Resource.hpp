#ifndef VADON_RENDER_GRAPHICSAPI_RESOURCE_RESOURCE_HPP
#define VADON_RENDER_GRAPHICSAPI_RESOURCE_RESOURCE_HPP
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
        TEXTURE_2D,
        TEXTURE_3D
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

    VADON_DECLARE_TYPED_POOL_HANDLE(Resource, ResourceHandle);

    template<typename T>
    struct TypedResourceHandle : public ResourceHandle
    {
        using _ResourceType = T;
        using _TypedHandle = TypedResourceHandle<T>;

        TypedResourceHandle<T>& operator=(const ResourceHandle& h) { handle = h.handle; return *this; }

        ResourceHandle to_resource_handle() const { return ResourceHandle{ .handle = this->handle }; }
        static TypedResourceHandle<T> from_resource_handle(ResourceHandle h) { _TypedHandle typed_handle; typed_handle.handle = h.handle; return typed_handle; }

        uint64_t to_uint() const { return this->handle.to_uint(); }
    };
}
#define VADON_GRAPHICSAPI_DECLARE_TYPED_RESOURCE_HANDLE(_resource, _name) using _name = Vadon::Render::TypedResourceHandle<_resource>
namespace Vadon::Utilities
{
    template<>
    struct EnableEnumBitwiseOperators<Vadon::Render::ResourceCPUAccessFlags> : public std::true_type
    {

    };
}
#endif