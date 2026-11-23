#ifndef VADON_RENDER_RHI_DEVICE_BARRIER_HPP
#define VADON_RENDER_RHI_DEVICE_BARRIER_HPP
#include <Vadon/Render/RHI/Resource/Buffer.hpp>
#include <Vadon/Render/RHI/Resource/Shader.hpp>
#include <Vadon/Render/RHI/Resource/Texture.hpp>
namespace Vadon::Render::RHI
{
	enum class AccessFlags
	{
		NONE = 0,
        
        INDEX_BUFFER = 1 << 0,
        VERTEX_BUFFER = 1 << 1,
        CONSTANT_BUFFER = 1 << 2,

        RENDER_TARGET_READ = 1 << 3,
        RENDER_TARGET_WRITE = 1 << 4,
        RENDER_TARGET = RENDER_TARGET_READ | RENDER_TARGET_WRITE,

        DEPTH_STENCIL_READ = 1 << 5,
        DEPTH_STENCIL_WRITE = 1 << 6,
        DEPTH_STENCIL = DEPTH_STENCIL_READ | DEPTH_STENCIL_WRITE,

        SHADER_RESOURCE = 1 << 7,
        READ_WRITE_RESOURCE = 1 << 8,

        COPY_SOURCE = 1 << 9,
        COPY_DESTINATION = 1 << 10,

        // TODO: multisampling?

        HOST_READ = 1 << 11,
        HOST_WRITE = 1 << 12
	};

    enum class ResourceLayout : uint8_t
    {
        UNDEFINED,
        GENERAL,
        PRESENT,

        RENDER_TARGET,
        DEPTH_STENCIL,
        DEPTH_READONLY_AND_STENCIL,
        DEPTH_AND_STENCIL_READONLY,
        DEPTH_STENCIL_READONLY,

        SHADER_RESOURCE,
        READ_WRITE_RESOURCE,

        COPY_SOURCE,
        COPY_DESTINATION

        // TODO: resolve
    };

    enum class PlaneFlags : uint8_t
    {
        ALL = 0,
        NONE = 1 << 7,

        COLOR = 1 << 0,

        DEPTH = 1 << 1,
        STENCIL = 1 << 2
    };

	struct AccessStage
	{
		AccessFlags access = AccessFlags::NONE;
		ShaderStage stages = ShaderStage::ALL;
	};

    struct AccessLayoutStage
    {
        AccessFlags access = AccessFlags::NONE;
        ResourceLayout layout = ResourceLayout::UNDEFINED;
        ShaderStage stages = ShaderStage::ALL;
    };

	struct BufferBarrier
	{
		BufferHandle buffer;
		AccessStage before;
		AccessStage after;
	};

	struct TextureBarrier
	{
        TextureHandle texture;
        AccessLayoutStage before;
        AccessLayoutStage after;
        uint16_t mip_offset = 0;
        uint16_t mip_count = 0;
        uint16_t layer_offset = 0;
        uint16_t layer_count = 0;
        PlaneFlags planes = PlaneFlags::ALL;
	};

	struct BarrierDescription
	{
        std::span<BufferBarrier> buffers;
        std::span<TextureBarrier> textures;
	};
}
#endif