#ifndef VADON_RENDER_RHI_COMMON_MEMORY_HPP
#define VADON_RENDER_RHI_COMMON_MEMORY_HPP
namespace Vadon::Render::RHI
{
	enum class MemoryLocation
	{
		DEVICE,
		UPLOAD,
		READBACK
	};
}
#endif