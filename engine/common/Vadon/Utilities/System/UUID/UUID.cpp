#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Core/Logger.hpp>

#include <Vadon/Utilities/Data/Encoding/Base64.hpp>
#include <Vadon/Utilities/Debugging/Assert.hpp>

// UUID generation implementation taken from: http://graemehill.ca/minimalist-cross-platform-uuid-guid-generation-in-c++/
#ifdef VADON_PLATFORM_WIN32
#include <objbase.h>
#elif defined(VADON_PLATFORM_LINUX)
#include <uuid/uuid.h>
#endif

#include <vector>

namespace
{
#ifdef VADON_PLATFORM_WIN32
    ::Vadon::Foundation::UUID generate_uuid_impl()
	{
		// TODO: report if error occurs?
		GUID guid;
		HRESULT result = CoCreateGuid(&guid);

        if (FAILED(result))
        {
            Vadon::Core::Logger::log_error("UUID: failed to generate GUID!\n");
            return ::Vadon::Foundation::UUID{};
        }

        return ::Vadon::Foundation::UUID{
            .data = {
                static_cast<unsigned char>((guid.Data1 >> 24) & 0xFF),
                static_cast<unsigned char>((guid.Data1 >> 16) & 0xFF),
                static_cast<unsigned char>((guid.Data1 >> 8) & 0xFF),
                static_cast<unsigned char>((guid.Data1) & 0xff),

                static_cast<unsigned char>((guid.Data2 >> 8) & 0xFF),
                static_cast<unsigned char>((guid.Data2) & 0xff),

                static_cast<unsigned char>((guid.Data3 >> 8) & 0xFF),
                static_cast<unsigned char>((guid.Data3) & 0xFF),

                guid.Data4[0],
                guid.Data4[1],
                guid.Data4[2],
                guid.Data4[3],
                guid.Data4[4],
                guid.Data4[5],
                guid.Data4[6],
                guid.Data4[7]
            }
        };
	}
#elif defined(VADON_PLATFORM_LINUX)
    Vadon::Utilities::UUID generate_uuid_impl()
    {
        Vadon::Utilities::UUID uuid;
        uuid_generate(uuid.data.data());
        return uuid;
    }
#else
    Vadon::Utilities::UUID generate_uuid_impl()
    {
        return Vadon::Utilities::UUID{};
    }
#endif
}

namespace Vadon::Utilities
{
	::Vadon::Foundation::UUID generate_uuid()
	{
		return generate_uuid_impl();
	}

    std::string uuid_to_hex_string(const ::Vadon::Foundation::UUID& uuid)
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

    std::string uuid_to_base64_string(const ::Vadon::Foundation::UUID& uuid)
    {
        return uuid.is_valid() ? Base64::encode(uuid.data) : "";
    }

    bool uuid_from_base64_string(std::string_view data_string, ::Vadon::Foundation::UUID& uuid)
    {
        // NOTE: empty is considered a valid parse as a "null" UUID
        if (data_string.empty())
        {
            uuid.invalidate();
            return true;
        }

        std::vector<unsigned char> decoded_data;
        if (Base64::decode(data_string, decoded_data) == false)
        {
            return false;
        }

        VADON_ASSERT(decoded_data.size() == ::Vadon::Foundation::UUID::c_uuid_width, "UUID decoded data is incorrect size!");
        std::copy(decoded_data.begin(), decoded_data.end(), uuid.data);
        return true;
    }
}