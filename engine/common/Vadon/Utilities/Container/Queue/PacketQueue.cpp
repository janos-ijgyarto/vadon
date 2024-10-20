#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Container/Queue/PacketQueue.hpp>

namespace Vadon::Utilities
{
	std::byte* PacketQueue::allocate_raw_data(uint32_t id, size_t size)
	{
		// FIXME: optimize this!
		{
			const Header packet_header{ .packet_id = id, .data_size = static_cast<uint32_t>(size) };
			const std::byte* header_begin = reinterpret_cast<const std::byte*>(&packet_header);
			const std::byte* header_end = header_begin + sizeof(Header);
 			m_data.insert(m_data.end(), header_begin, header_end);
		}

		const size_t data_offset = m_data.size();
		m_data.insert(m_data.end(), size, std::byte(0));
		return m_data.data() + data_offset;
	}
}