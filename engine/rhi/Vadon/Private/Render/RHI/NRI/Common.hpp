#ifndef VADON_PRIVATE_RENDER_RHI_NRI_COMMON_HPP
#define VADON_PRIVATE_RENDER_RHI_NRI_COMMON_HPP
#include <Vadon/Utilities/Debugging/Assert.hpp>
#include <nri/NRIDescs.h>
namespace Vadon::Private::Render::RHI::NRI
{
}
#define NRI_ABORT_ON_FAILURE(result) \
    if ((result) != nri::Result::SUCCESS) { \
        VADON_ERROR("NRI failure!"); \
    }
#endif