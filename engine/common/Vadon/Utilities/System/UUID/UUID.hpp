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
	VADONCOMMON_API std::string uuid_to_string(const ::Vadon::Foundation::UUID& uuid);
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
}
#endif