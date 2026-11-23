#ifndef VADON_RENDER_RHI_DEVICE_QUEUE_HPP
#define VADON_RENDER_RHI_DEVICE_QUEUE_HPP
#include <Vadon/Render/RHI/Device/Fence.hpp>
#include <Vadon/Render/RHI/Resource/CommandList.hpp>
#include <span>
namespace Vadon::Render::RHI
{
	enum class QueueType
	{
		GRAPHICS,
		COMPUTE,
		COPY
	};

	struct FenceSubmitDescription
	{
		FenceHandle fence;
		uint64_t value;
		// TODO: stage bits (relevant on Vulkan!)
	};

	struct QueueSubmitDescription
	{
		std::span<FenceSubmitDescription> wait_fences;
		std::span<const CommandListHandle> command_lists;
		std::span<FenceSubmitDescription> signal_fences;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Queue, QueueHandle);
}
#endif