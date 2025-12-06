#ifndef VADONEDITOR_NETWORK_MESSAGE_MESSAGE_HPP
#define VADONEDITOR_NETWORK_MESSAGE_MESSAGE_HPP
#include <cstdint>
namespace VadonEditor::Network
{
	struct MessageHeader
	{
		uint64_t category : 16; // NOTE: after reading the category and type, we can figure out how to parse the rest of the message
		uint64_t type : 16;
		uint64_t size : 32; // Read this many bytes, then advance to next message
	};

	enum class MessageCategory
	{
		MODEL,
		WINDOW,
		TEST
		// TODO: other messages?
	};

	template<MessageCategory C, uint64_t T>
	struct MessageAttribute
	{
		static constexpr MessageCategory c_category = C;
		static constexpr uint64_t c_type = T;
	};

	struct TestMessage : public MessageAttribute<MessageCategory::TEST, 0>
	{
		int number;
		float other_number;
	};
}
#endif