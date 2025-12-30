#ifndef VADON_FOUNDATION_UUID_HPP
#define VADON_FOUNDATION_UUID_HPP
#include <Vadon/Foundation/Utilities/Numeric.hpp>
namespace Vadon
{
	namespace Foundation
	{
		using UUIDDataType = unsigned char;

		struct UUID
		{
			static constexpr size_t c_uuid_width = 16;
			static constexpr UUIDDataType c_invalid_uuid[c_uuid_width] = { 0 };

			UUIDDataType data[c_uuid_width] = { 0 };

			bool operator==(const UUID& other) const
			{
				return std::memcmp(data, other.data, c_uuid_width) == 0;
			}

			bool is_valid() const
			{
				return std::memcmp(data, c_invalid_uuid, c_uuid_width) != 0;
			}

			void invalidate() { memset(data, 0, c_uuid_width); }
		};
	}
}

// Specialize std::hash to allow use in std::unordered_map
namespace std 
{
	template<> struct hash<::Vadon::Foundation::UUID>
	{
		size_t operator()(const ::Vadon::Foundation::UUID& uuid) const noexcept {
			const ::Vadon::Foundation::uint64* p = reinterpret_cast<const ::Vadon::Foundation::uint64*>(uuid.data);
			std::hash<::Vadon::Foundation::uint64> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}
#endif