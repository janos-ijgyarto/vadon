#ifndef VADON_UTILITIES_SYSTEM_UUID_UUID_HPP
#define VADON_UTILITIES_SYSTEM_UUID_UUID_HPP
#include <Vadon/Foundation/Utilities/UUID.hpp>
#include <Vadon/Common.hpp>
#include <string>
namespace Vadon::Utilities
{
	// TODO: create UUID wrapper object which also contains a debug string!
	// AND/OR have some kind of debug lookup where the UUID is mapped to a debug string!

	VADONCOMMON_API ::Vadon::Foundation::UUID generate_uuid();

	// FIXME: these will always produce the same length string
	// replace with a type that avoids dynamic allocation?

	constexpr char uuid_half_byte_to_hex(const char c)
	{
		if (c < 10)
		{
			return char('0' + c);
		}
		else
		{
			return char('A' + (c - 10));
		}
	}

	constexpr ::Vadon::Foundation::UUIDString uuid_to_string(const ::Vadon::Foundation::UUID& uuid)
	{
		::Vadon::Foundation::UUIDString result_data = {};

		constexpr char c_separator = '-';
		constexpr size_t c_separator_offsets[4] = { 8, 13, 18, 23 };

		constexpr char half_byte_mask = ((1 << 4) - 1);
		size_t char_index = 0;
		size_t separator_index = 0;
		for (size_t index = 0; index < ::Vadon::Foundation::UUID::c_uuid_width; ++index)
		{
			if ((separator_index < 4) && (char_index == c_separator_offsets[separator_index]))
			{
				// Write separator
				result_data.string[char_index] = c_separator;
				++char_index;
				++separator_index;
			}

			const char uuid_byte = uuid.data[index];

			result_data.string[char_index] = uuid_half_byte_to_hex((uuid_byte >> 4) & half_byte_mask);
			result_data.string[char_index + 1] = uuid_half_byte_to_hex(uuid_byte & half_byte_mask);
			char_index += 2;
		}

		return result_data;
	}

	constexpr std::string uuid_to_hex_string(const ::Vadon::Foundation::UUID& uuid)
	{
		std::string uuid_string;
		uuid_string.resize(::Vadon::Foundation::UUID::c_uuid_width * 2);

		constexpr auto half_byte_to_hex = +[](const char c) {
			if (c < 10)
			{
				return char('0' + c);
			}
			else
			{
				return char('A' + (c - 10));
			}
			};

		constexpr char half_byte_mask = ((1 << 4) - 1);
		size_t char_index = 0;
		for (size_t index = 0; index < ::Vadon::Foundation::UUID::c_uuid_width; ++index)
		{
			const char uuid_byte = uuid.data[index];

			uuid_string[char_index] = half_byte_to_hex((uuid_byte >> 4) & half_byte_mask);
			uuid_string[char_index + 1] = half_byte_to_hex(uuid_byte & half_byte_mask);
			char_index += 2;
		}

		return uuid_string;
	}

	VADONCOMMON_API std::string uuid_to_base64_string(const ::Vadon::Foundation::UUID& uuid);
	VADONCOMMON_API bool uuid_from_base64_string(std::string_view data_string, ::Vadon::Foundation::UUID& uuid);

	// Source: https://stackoverflow.com/a/79657606
	template <std::size_t N>
	struct UUIDLiteral 
	{
		static_assert(N == 37,  // buffer contains \0
			"UUID literal must be exactly 36 chars (8-4-4-4-12)");

		Vadon::Foundation::UUID result{};

		constexpr UUIDLiteral(char const (&str)[N])
		{
			// dash checks - can't use static_assert
			if (!(str[8] == '-' && str[13] == '-' && str[18] == '-' &&
				str[23] == '-'))
				throw "Dashes must be at 8,13,18,23";

			// constexpr lambda to convert one hex char -> 0..15
			constexpr auto hex_val = [](const char c) {
				return static_cast<Vadon::Foundation::UUIDDataType>(
					(c >= '0' && c <= '9') ? (c - '0') :
					(c >= 'a' && c <= 'f') ? (10 + (c - 'a')) :
					(c >= 'A' && c <= 'F') ? (10 + (c - 'A')) :
					throw "Only 0-9,A-F or 0-9,a-f is allowed in uuid expression."
					);
				};

			auto idx = 0;
			for (Vadon::Foundation::UUIDDataType& byte : result.data) {
				// skip dash if present
				if (str[idx] == '-') ++idx;
				auto hi = hex_val(str[idx++]);

				if (str[idx] == '-') ++idx;
				auto lo = hex_val(str[idx++]);

				byte = static_cast<Vadon::Foundation::UUIDDataType>((hi << 4) | lo);
			}
		}
	};

	template <UUIDLiteral U>
	constexpr ::Vadon::Foundation::UUID operator"" _uuid()
	{
		return U.result;
	}

	constexpr ::Vadon::Foundation::UUID string_to_uuid(const ::Vadon::Foundation::UUIDString& string)
	{
		return UUIDLiteral(string.string).result;
	}

	inline std::string sanitize_uuid_label(std::string_view input)
	{
		std::string output;
		output.reserve(input.size());

		for (char current_char : input)
		{
			if (std::isalpha(current_char) != 0)
			{
				output.push_back(static_cast<char>(std::tolower(current_char)));
			}
			else
			{
				output.push_back('_');
			}
		}

		return output;
	}

	inline std::string serialize_labeled_uuid(std::string_view label, const ::Vadon::Foundation::UUID& uuid)
	{
		const std::string sanitized_label = sanitize_uuid_label(label);
		return sanitized_label + "|" + uuid_to_base64_string(uuid);
	}

	inline ::Vadon::Foundation::UUID parse_labeled_uuid(std::string_view uuid_string)
	{
		const size_t separator_index = uuid_string.find('|');
		::Vadon::Foundation::UUID output;
		if (separator_index != std::string::npos)
		{
			uuid_from_base64_string(uuid_string.substr(separator_index + 1), output);
		}
		else
		{
			// Assume it's a regular UUID string
			uuid_from_base64_string(uuid_string, output);
		}

		return output;
	}
}
#endif