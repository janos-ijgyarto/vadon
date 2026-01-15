#ifndef VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_MESSAGE_HPP
#define VADON_FOUNDATION_EDITOR_NETWORK_MESSAGE_MESSAGE_HPP
#include <Vadon/Foundation/Utilities/Numeric.hpp>
namespace Vadon
{
	namespace Foundation
	{
		enum class EditorMessageCategory
		{
			SIMULATOR,
			MODEL,
			PLATFORM,
			TEST
			// TODO: other messages?
		};

		struct EditorMessageTest
		{
			int number;
			float other_number;
		};

		class EditorMessageReader
		{
		public:
			EditorMessageReader(const void* data, size_t size)
				: m_data(data)
				, m_size(size)
				, m_current_offset(0)
			{

			}

			static constexpr size_t c_header_size = sizeof(::Vadon::Foundation::EditorMessageCategory) + sizeof(uint32);

			::Vadon::Foundation::EditorMessageCategory get_current_category() const { return *reinterpret_cast<const ::Vadon::Foundation::EditorMessageCategory*>(reinterpret_cast<const char*>(m_data) + m_current_offset); }

			uint32 get_message_data_size() const { return *reinterpret_cast<const uint32*>(reinterpret_cast<const char*>(m_data) + m_current_offset + sizeof(::Vadon::Foundation::EditorMessageCategory)); }

			const char* get_current_message_data() const { return reinterpret_cast<const char*>(m_data) + m_current_offset + c_header_size; }

			void next_message()
			{
				if (is_valid() == false)
				{
					return;
				}

				const uint32 message_size = get_message_data_size();
				m_current_offset += message_size + c_header_size;
			}

			bool is_valid() const { return m_current_offset < m_size; }
			size_t get_size() const { return m_size; }
		private:
			const void* m_data;
			size_t m_size;
			size_t m_current_offset;
		};
	}
}
#endif