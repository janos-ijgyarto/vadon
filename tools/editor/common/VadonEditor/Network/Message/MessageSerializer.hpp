#ifndef VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#define VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#include <VadonEditor/Common.hpp>
#include <Vadon/Foundation/Editor/Message.hpp>
#include <vector>
namespace VadonEditor::Network
{
	// FIXME: move to Foundation so the engine-side code can also use this!
	class MessageSerializer
	{
	public:
		VADONEDITORCOMMON_API static void serialize_message_trivial(const void* message_data, size_t message_size, std::vector<char>& dest_buffer);

		template<typename T>
		static void serialize_message(const T& message, std::vector<char>& dest_buffer)
		{
			serialize_message_trivial(&message, sizeof(T), dest_buffer);
		}

		VADONEDITORCOMMON_API static void deserialize_message_trivial(void* message_data, size_t message_size, const void* source_data);

		template<typename T>
		static void deserialize_message(T& message, const void* source_data)
		{
			deserialize_message_trivial(&message, sizeof(T), source_data);
		}

		template<typename T>
		static void write_message(const T& message, std::vector<char>& buffer)
		{
			using MessageTypeTrait = Vadon::Foundation::EditorMessageTypeTrait<T>;

			write_message_header(MessageTypeTrait::get_category(), MessageTypeTrait::get_type(), buffer);
			serialize_message(message, buffer);
			fixup_message_size(buffer);
		}

		template<typename T>
		static void parse_message(const ::Vadon::Foundation::EditorMessageHeader& /*header*/, const void* message_data, T& out_message)
		{
			// TODO: ensure header matches!
			deserialize_message(out_message, message_data);
		}
	private:
		VADONEDITORCOMMON_API static void write_message_header(::Vadon::Foundation::EditorMessageCategory category, uint64_t type, std::vector<char>& buffer);
		VADONEDITORCOMMON_API static void fixup_message_size(std::vector<char>& buffer);
	};
}
#endif