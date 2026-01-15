#ifndef VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#define VADONEDITOR_NETWORK_MESSAGE_MESSAGESERIALIZER_HPP
#include <VadonEditor/Common.hpp>
#include <Vadon/Foundation/Editor/Network/Message/Message.hpp>
#include <vector>
namespace VadonEditor::Network
{
	// FIXME: move to Foundation to be useable from both editor and engine?
	class MessageSerializer
	{
	public:
		VADONEDITORCOMMON_API void write_message(::Vadon::Foundation::EditorMessageCategory category, const void* message_data, size_t message_size);

		template<typename T>
		void write_message_trivial(::Vadon::Foundation::EditorMessageCategory category, const T& message)
		{
			write_message(category, &message, sizeof(T));
		}

		const std::vector<char>& get_buffer() const { return m_buffer; }
	private:
		std::vector<char> m_buffer;
	};
}
#endif