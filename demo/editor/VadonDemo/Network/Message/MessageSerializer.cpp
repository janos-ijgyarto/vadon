#include <VadonDemo/Network/Message/MessageSerializer.hpp>

namespace VadonDemo::Network
{
	void MessageSerializer::write_message(::Vadon::Foundation::EditorMessageCategory category, const void* message_data, size_t message_size)
	{
		const size_t combined_size = sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t) + message_size;
		const size_t prev_buffer_size = m_buffer.size();
		
		m_buffer.insert(m_buffer.end(), combined_size, 0);
		
		memcpy(m_buffer.data() + prev_buffer_size, &category, sizeof(::Vadon::Foundation::EditorMessageCategory));

		const uint32_t message_size_uint = static_cast<uint32_t>(message_size);
		memcpy(m_buffer.data() + prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory), &message_size_uint, sizeof(uint32_t));
		
		memcpy(m_buffer.data() + prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t), message_data, message_size);
	}
}