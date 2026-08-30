#include <VadonEditor/Network/Message/MessageSerializer.hpp>

namespace VadonEditor::Network
{
	char* MessageSerializer::allocate_message(::Vadon::Foundation::EditorMessageCategory category, size_t message_size)
	{
		const size_t combined_size = Vadon::Foundation::EditorMessageReader::c_header_size + message_size;
		const size_t prev_buffer_size = m_buffer.size();
		
		m_buffer.insert(m_buffer.end(), combined_size, 0);
		
		memcpy(m_buffer.data() + prev_buffer_size, &category, sizeof(::Vadon::Foundation::EditorMessageCategory));

		const uint32_t message_size_uint = static_cast<uint32_t>(message_size);
		memcpy(m_buffer.data() + prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory), &message_size_uint, sizeof(uint32_t));
		
		// Return pointer to where client can allocate the message data
		return m_buffer.data() + prev_buffer_size + Vadon::Foundation::EditorMessageReader::c_header_size;
	}
}