#ifndef VADON_RENDER_RHI_RESOURCE_SAMPLER_HPP
#define VADON_RENDER_RHI_RESOURCE_SAMPLER_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render::RHI
{
    enum class SamplerFilter
    {
        NEAREST,
        LINEAR
    };

    enum class SamplerFilterOp
    {
        AVERAGE,
        MIN,
        MAX
    };

    struct SamplerFilters
    {
        SamplerFilter min = SamplerFilter::NEAREST;
        SamplerFilter mag = SamplerFilter::NEAREST;
        SamplerFilter mip = SamplerFilter::NEAREST;
        SamplerFilterOp op = SamplerFilterOp::AVERAGE;
    };

    enum class SamplerAddressMode
    {
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_EDGE,
        CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE,
    };

    struct SamplerAddressModes
    {
        SamplerAddressMode u = SamplerAddressMode::REPEAT;
        SamplerAddressMode v = SamplerAddressMode::REPEAT;
        SamplerAddressMode w = SamplerAddressMode::REPEAT;
    };

	struct SamplerDescription
	{
        SamplerAddressModes address_modes = {};
        SamplerFilters filters = {};
        uint8_t anisotropy = 0;
        float mip_bias = 0.f;
	};

    VADON_DECLARE_TYPED_POOL_HANDLE(Sampler, SamplerHandle);
}
#endif