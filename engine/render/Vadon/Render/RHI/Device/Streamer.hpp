#ifndef VADON_RENDER_RHI_DEVICE_STREAMER_HPP
#define VADON_RENDER_RHI_DEVICE_STREAMER_HPP
#include <Vadon/Render/RHI/Resource/Buffer.hpp>
#include <Vadon/Render/RHI/Resource/Texture.hpp>
#include <span>
namespace Vadon::Render::RHI
{
	struct StreamerDescription
	{
		uint32_t constant_buffer_size = 0;
		BufferDescription dynamic_buffer_description;
		uint32_t queued_frames = 1;
		uint32_t capacity = 0;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Streamer, StreamerHandle);

	using StreamerCopyBatch = uint64_t;

	struct DataBatch
	{
		void* data;
		uint64_t size;
	};

	struct StreamBufferDataDescription
	{
		std::span<DataBatch> batches;
		uint32_t alignment;

		StreamerCopyBatch copy_batch;
		BufferHandle destination_buffer;
		uint64_t destination_offset;
	};

	struct StreamTextureDataDescription
	{
		const void* data;
		uint32_t data_row_pitch;
		uint32_t data_slice_pitch;

		StreamerCopyBatch copy_batch;
		TextureHandle destination_texture;
		// TODO: region
	};
}
#endif