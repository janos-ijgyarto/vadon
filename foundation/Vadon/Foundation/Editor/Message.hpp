#ifndef VADON_FOUNDATION_EDITOR_MESSAGE_HPP
#define VADON_FOUNDATION_EDITOR_MESSAGE_HPP
#include <Vadon/Foundation/Utilities/Numeric.hpp>
namespace Vadon
{
	namespace Foundation
	{
		struct EditorMessageHeader
		{
			uint64 category : 16; // NOTE: after reading the category and type, we can figure out how to parse the rest of the message
			uint64 type : 16;
			uint64 size : 32; // Read this many bytes, then advance to next message
		};

		enum class EditorMessageCategory
		{
			MODEL,
			WINDOW,
			TEST
			// TODO: other messages?
		};

		template<typename T>
		struct EditorMessageTypeTrait
		{
			static constexpr EditorMessageCategory get_category()
			{
				static_assert(false, "Message not registered!");
			}

			static constexpr uint64 get_type()
			{
				static_assert(false, "Message not registered!");
			}
		};
	}
}

#define VADON_DECLARE_EDITOR_MESSAGE_TYPE_INFO(_type, _category, _message_type) template<>\
struct ::Vadon::Foundation::EditorMessageTypeTrait<_type>\
{\
	static constexpr ::Vadon::Foundation::EditorMessageCategory get_category()\
	{\
		return _category;\
	}\
	static constexpr uint64 get_type()\
	{\
		return _message_type;\
	}\
}

namespace Vadon
{
	namespace Foundation
	{
		struct EditorTestMessage
		{
			int number;
			float other_number;
		};

		VADON_DECLARE_EDITOR_MESSAGE_TYPE_INFO(EditorTestMessage, EditorMessageCategory::TEST, 0);
	}
}
#endif