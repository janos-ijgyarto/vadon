#include <VadonEditor/Network/Message/MessageSerializer.hpp>

namespace VadonEditor::Network
{
	char* MessageSerializer::allocate_message(::Vadon::Foundation::EditorMessageCategory category, size_t message_size)
	{
		const size_t combined_size = sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t) + message_size;
		const size_t prev_buffer_size = m_buffer.size();

		// Allocate the required memory
		m_buffer.insert(m_buffer.end(), combined_size, 0);

		// Add the header (category and size)
		memcpy(m_buffer.data() + prev_buffer_size, &category, sizeof(::Vadon::Foundation::EditorMessageCategory));

		const uint32_t message_size_uint = static_cast<uint32_t>(message_size);
		memcpy(m_buffer.data() + prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory), &message_size_uint, sizeof(uint32_t));

		// Return the location to write to
		return m_buffer.data() + prev_buffer_size + +sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t);
	}

	void MessageSerializer::write_message(::Vadon::Foundation::EditorMessageCategory category, const void* message_data, size_t message_size)
	{
		char* write_address = allocate_message(category, message_size);
		memcpy(write_address, message_data, message_size);
	}
}