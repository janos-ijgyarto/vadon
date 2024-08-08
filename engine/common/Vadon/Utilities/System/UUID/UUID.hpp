#ifndef VADON_UTILITIES_SYSTEM_UUID_UUID_HPP
#define VADON_UTILITIES_SYSTEM_UUID_UUID_HPP
#include <Vadon/Common.hpp>
#include <array>
namespace Vadon::Utilities
{
	class Serializer;

	// FIXME: use std::byte?
	using UUIDData = std::array<unsigned char, 16>;

	struct UUID
	{
		static constexpr UUIDData c_invalid_uuid_data = {};

		UUIDData data = c_invalid_uuid_data;

		VADONCOMMON_API static UUID generate();

		bool operator==(const UUID& other) const
		{
			return std::memcmp(data.data(), other.data.data(), data.size()) == 0;
		}

		bool is_valid() const
		{
			return std::memcmp(data.data(), c_invalid_uuid_data.data(), data.size()) != 0;
		}

		VADONCOMMON_API std::string to_base64_string() const;
		VADONCOMMON_API bool from_base64_string(std::string_view data_string);

		VADONCOMMON_API bool serialize(Serializer& serializer, std::string_view key);
		VADONCOMMON_API bool serialize(Serializer& serializer, size_t index);
	};
}

// Specialize std::hash to allow use in std::unordered_map
namespace std {
	template<> struct hash<::Vadon::Utilities::UUID>
	{
		size_t operator()(const ::Vadon::Utilities::UUID& uuid) const noexcept {
			const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(uuid.data.data());
			std::hash<std::uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}
#endif