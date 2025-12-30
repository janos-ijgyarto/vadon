#include <VadonEditor/Network/Message/MessageSerializer.hpp>

namespace VadonEditor::Network
{
	void MessageSerializer::serialize_message_trivial(const void* message_data, size_t message_size, std::vector<char>& dest_buffer)
	{
		const char* message_data_ptr = reinterpret_cast<const char*>(message_data);
		dest_buffer.insert(dest_buffer.end(), message_data_ptr, message_data_ptr + message_size);
	}

	void MessageSerializer::deserialize_message_trivial(void* message_data, size_t message_size, const void* source_data)
	{
		memcpy(message_data, source_data, message_size);
	}

	void MessageSerializer::write_message_header(::Vadon::Foundation::EditorMessageCategory category, uint64_t type, std::vector<char>& buffer)
	{
		::Vadon::Foundation::EditorMessageHeader header;
		header.category = static_cast<uint64_t>(category);
		header.type = type;
		header.size = 0;

		const char* header_ptr = reinterpret_cast<char*>(&header);

		buffer.insert(buffer.end(), header_ptr, header_ptr + sizeof(::Vadon::Foundation::EditorMessageHeader));
	}

	void MessageSerializer::fixup_message_size(std::vector<char>& buffer)
	{
		// TODO: assert if invalid
		::Vadon::Foundation::EditorMessageHeader* header = reinterpret_cast<::Vadon::Foundation::EditorMessageHeader*>(buffer.data());
		header->size = buffer.size() - sizeof(::Vadon::Foundation::EditorMessageHeader);
	}
}