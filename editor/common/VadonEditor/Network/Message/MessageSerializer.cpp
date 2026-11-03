#include <VadonEditor/Network/Message/MessageSerializer.hpp>

namespace VadonEditor::Network
{
	void MessageSerializer::write_message(::Vadon::Foundation::EditorMessageCategory category, const void* message_data, size_t message_size)
	{
		// Allocase space for the category, size, and the message itself
		const size_t combined_size = sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t) + message_size;

		// Store the previous buffer end
		m_prev_buffer_size = m_buffer.size();

		// Allocate the required memory
		m_buffer.insert(m_buffer.end(), combined_size, 0);

		// Add the header (category and size)
		memcpy(m_buffer.data() + m_prev_buffer_size, &category, sizeof(::Vadon::Foundation::EditorMessageCategory));

		const uint32_t message_size_uint = static_cast<uint32_t>(message_size);
		set_current_message_size(message_size_uint);

		void* data_dst = get_current_message_ptr();
		memcpy(data_dst, message_data, message_size);
	}

	::Vadon::Foundation::EditorMessageRawDataEntry MessageSerializer::append_message_data(const void* data, size_t size)
	{
		::Vadon::Foundation::EditorMessageRawDataEntry data_entry;
		data_entry.offset = static_cast<uint32_t>(m_buffer.size());
		data_entry.size = static_cast<uint32_t>(size);

		const char* data_begin = static_cast<const char*>(data);
		const char* data_end = data_begin + size;

		m_buffer.insert(m_buffer.end(), data_begin, data_end);

		const uint32_t prev_message_size = get_current_message_size();
		set_current_message_size(prev_message_size + data_entry.size);

		return data_entry;
	}

	void MessageSerializer::set_current_message_size(uint32_t size)
	{
		memcpy(m_buffer.data() + m_prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory), &size, sizeof(uint32_t));
	}

	uint32_t MessageSerializer::get_current_message_size() const
	{
		const void* read_address = m_buffer.data() + m_prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory);
		return *static_cast<const uint32_t*>(read_address);
	}
}