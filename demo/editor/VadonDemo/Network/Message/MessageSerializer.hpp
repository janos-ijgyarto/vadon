#ifndef VADONDEMO_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#define VADONDEMO_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <vector>
namespace VadonDemo::Network
{
	// FIXME: move to Foundation to be useable from both editor and engine?
	class MessageSerializer
	{
	public:
		char* allocate_message(::Vadon::Foundation::EditorMessageCategory category, size_t message_size);

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