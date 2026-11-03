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
		using RawDataEntry = ::Vadon::Foundation::EditorMessageRawDataEntry;

		VADONEDITOR_API void write_message(::Vadon::Foundation::EditorMessageCategory category, const void* message_data, size_t message_size);

		template<typename T>
		void write_message(::Vadon::Foundation::EditorMessageCategory category, const T& message)
		{
			write_message(category, &message, sizeof(T));
		}

		VADONEDITOR_API::Vadon::Foundation::EditorMessageRawDataEntry append_message_data(const void* data, size_t size);

		template<typename T>
		T* get_current_message()
		{
			return static_cast<T*>(get_current_message_ptr());
		}

		template<typename M>
		::Vadon::Foundation::EditorMessageRawDataEntry append_message_raw_data(const void* data, size_t size, RawDataEntry M::* entry_member = nullptr)
		{
			::Vadon::Foundation::EditorMessageRawDataEntry entry = append_message_data(data, size);
			if (entry_member != nullptr)
			{
				get_current_message<M>()->*entry_member = entry;
			}

			return entry;
		}

		template<typename M, typename T>
		::Vadon::Foundation::EditorMessageRawDataEntry append_message_data(const T& data, RawDataEntry M::* entry_member = nullptr)
		{
			return append_message_raw_data<M>(&data, sizeof(T), entry_member);
		}

		void* get_message_raw_data(const ::Vadon::Foundation::EditorMessageRawDataEntry& entry)
		{
			return m_buffer.data() + entry.offset;
		}

		template<typename T>
		T* get_message_data(const ::Vadon::Foundation::EditorMessageRawDataEntry& entry)
		{
			static_cast<T*>(get_message_raw_data(entry));
		}

		const std::vector<char>& get_buffer() const { return m_buffer; }
		void clear() { m_buffer.clear(); }
	private:
		void* get_current_message_ptr() { return m_buffer.data() + m_prev_buffer_size + sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32_t); }

		void set_current_message_size(uint32_t size);
		uint32_t get_current_message_size() const;

		std::vector<char> m_buffer;
		size_t m_prev_buffer_size = 0;
	};
}
#endif