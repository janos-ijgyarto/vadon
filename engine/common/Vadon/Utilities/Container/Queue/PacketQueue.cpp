#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

namespace Vadon::Utilities
{
	void PacketQueue::Iterator::advance(size_t block_size)
	{
		m_data_it += sizeof(uint32_t) + block_size;
	}

	uint32_t PacketQueue::Iterator::get_header_id() const
	{
		// TODO: assert if invalid!
		uint32_t id = 0;
		memcpy(&id, std::to_address(m_data_it), sizeof(uint32_t));

		return id;
	}

	const std::byte* PacketQueue::Iterator::get_packet_data() const
	{
		// TODO: assert if invalid!
		return std::to_address(m_data_it) + sizeof(uint32_t);
	}

	std::byte* PacketQueue::allocate_raw_data(uint32_t id, size_t size)
	{
		// FIXME: optimize this!
		{
			const std::byte* id_begin = reinterpret_cast<const std::byte*>(&id);
			const std::byte* id_end = id_begin + sizeof(uint32_t);
 			m_data.insert(m_data.end(), id_begin, id_end);
		}

		const size_t data_offset = m_data.size();
		m_data.insert(m_data.end(), size, std::byte(0));
		return m_data.data() + data_offset;
	}
}