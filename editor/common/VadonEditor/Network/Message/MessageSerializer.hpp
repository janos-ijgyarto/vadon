#ifndef VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#define VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <vector>
namespace VadonEditor::Network
{
	class MessageSerializer
	{
	public:
		VADONEDITOR_API char* allocate_message(::Vadon::Foundation::EditorMessageCategory category, size_t message_size);

		template<typename T>
		void write_message_trivial(::Vadon::Foundation::EditorMessageCategory category, const T& message)
		{
			char* message_data = allocate_message(category, sizeof(T));
			memcpy(message_data, &message, sizeof(T));
		}

		const std::vector<char>& get_buffer() const { return m_buffer; }
	private:
		std::vector<char> m_buffer;
	};
}
#endif