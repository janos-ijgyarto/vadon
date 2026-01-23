#ifndef VADON_UTILITIES_DATA_ENCODING_BASE64_HPP
#define VADON_UTILITIES_DATA_ENCODING_BASE64_HPP
#include <span>
#include <string>
#include <vector>

namespace Vadon::Utilities
{
    class Base64 
    {
    public:
        // TODO: have "stream" versions?
        // FIXME: use std::byte?
        static std::string encode(std::span<const unsigned char> data);
        static bool decode(std::string_view input, std::vector<unsigned char>& result);
    };
}
#endif