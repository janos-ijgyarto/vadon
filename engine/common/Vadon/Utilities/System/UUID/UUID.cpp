#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/System/UUID/UUID.hpp>

#include <Vadon/Core/Logger.hpp>
#include <Vadon/Utilities/Data/Encoding/Base64.hpp>

// UUID generation implementation taken from: http://graemehill.ca/minimalist-cross-platform-uuid-guid-generation-in-c++/
#ifdef VADON_PLATFORM_WIN32
#include <objbase.h>
#elif defined(VADON_PLATFORM_LINUX)
#include <uuid/uuid.h>
#endif

namespace
{
#ifdef VADON_PLATFORM_WIN32
    Vadon::Utilities::UUID generate_uuid_impl()
	{
		// TODO: report if error occurs?
		GUID guid;
		HRESULT result = CoCreateGuid(&guid);

        if (FAILED(result))
        {
            Vadon::Core::Logger::log_error("UUID: failed to generate GUID!\n");
            return Vadon::Utilities::UUID{};
        }

        return Vadon::Utilities::UUID{
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
	UUID UUID::generate()
	{
		return generate_uuid_impl();
	}

    std::string UUID::to_base64_string() const
    {
        return is_valid() ? Base64::encode(data) : "";
    }

    bool UUID::from_base64_string(std::string_view data_string)
    {
        if (data_string.empty())
        {
            invalidate();
            return true;
        }

        std::vector<unsigned char> decoded_data;
        if (Base64::decode(data_string, decoded_data) == false)
        {
            return false;
        }

        assert(decoded_data.size() == data.size());
        std::copy(decoded_data.begin(), decoded_data.end(), data.begin());
        return true;
    }
}