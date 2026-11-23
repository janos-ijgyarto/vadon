#ifndef VADON_RENDER_RHI_DEVICE_FENCE_HPP
#define VADON_RENDER_RHI_DEVICE_FENCE_HPP
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
namespace Vadon::Render::RHI
{
	struct FenceDescription
	{
		uint64_t initial_value = 0;
		bool shared = false;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Fence, FenceHandle);
}
#endif