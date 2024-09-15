#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

namespace Vadon::Utilities
{
	void PacketQueue::Iterator::next()
	{
		// TODO: assert if invalid!
		const size_t size = get_packet_size();
		m_data_it += size + sizeof(uint32_t) + sizeof(size_t);
	}

	uint32_t PacketQueue::Iterator::get_header_id() const
	{
		// TODO: assert if invalid!
		uint32_t id = 0;
		memcpy(&id, std::to_address(m_data_it), sizeof(uint32_t));

		return id;
	}

	size_t PacketQueue::Iterator::get_packet_size() const
	{
		// TODO: assert if invalid!
		size_t size = 0;
		memcpy(&size, std::to_address(m_data_it) + sizeof(uint32_t), sizeof(size_t));

		return size;
	}

	const std::byte* PacketQueue::Iterator::get_packet_data() const
	{
		// TODO: assert if invalid!
		return std::to_address(m_data_it) + sizeof(uint32_t) + sizeof(size_t);
	}

	void PacketQueue::internal_write_data(uint32_t id, const std::byte* data, size_t size)
	{
		{
			const std::byte* id_begin = reinterpret_cast<const std::byte*>(&id);
			const std::byte* id_end = id_begin + sizeof(uint32_t);
 			m_data.insert(m_data.end(), id_begin, id_end);
		}

		{
			const std::byte* size_begin = reinterpret_cast<const std::byte*>(&size);
			const std::byte* size_end = size_begin + sizeof(size_t);
			m_data.insert(m_data.end(), size_begin, size_end);
		}

		{
			const std::byte* data_begin = reinterpret_cast<const std::byte*>(data);
			const std::byte* data_end = data_begin + size;
			m_data.insert(m_data.end(), data_begin, data_end);
		}
	}
}