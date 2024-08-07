#include <Vadon/Private/PCH/Common.hpp>
#include <Vadon/Utilities/Data/Encoding/Base64.hpp>

#include <Vadon/Core/Logger.hpp>

namespace Vadon::Utilities
{
    // Implementation source: https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
    /**
         * The MIT License (MIT)
         * Copyright (c) 2016-2024 tomykaira
         *
         * Permission is hereby granted, free of charge, to any person obtaining
         * a copy of this software and associated documentation files (the
         * "Software"), to deal in the Software without restriction, including
         * without limitation the rights to use, copy, modify, merge, publish,
         * distribute, sublicense, and/or sell copies of the Software, and to
         * permit persons to whom the Software is furnished to do so, subject to
         * the following conditions:
         *
         * The above copyright notice and this permission notice shall be
         * included in all copies or substantial portions of the Software.
         *
         * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
         * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
         * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
         * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
         * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
         * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
         * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    */
    std::string Base64::encode(std::span<const unsigned char> data)
    {
        static constexpr const char c_encoding_table[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' 
        };

        size_t in_len = data.size();
        size_t out_len = 4 * ((in_len + 2) / 3);
        std::string ret(out_len, '\0');
        size_t i;
        char* p = const_cast<char*>(ret.c_str());

        for (i = 0; (in_len > 2) && (i < (in_len - 2)); i += 3)
        {
            *p++ = c_encoding_table[(data[i] >> 2) & 0x3F];
            *p++ = c_encoding_table[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
            *p++ = c_encoding_table[((data[i + 1] & 0xF) << 2) | ((int)(data[i + 2] & 0xC0) >> 6)];
            *p++ = c_encoding_table[data[i + 2] & 0x3F];
        }
        if (i < in_len) 
        {
            *p++ = c_encoding_table[(data[i] >> 2) & 0x3F];
            if (i == (in_len - 1))
            {
                *p++ = c_encoding_table[((data[i] & 0x3) << 4)];
                *p++ = '=';
            }
            else
            {
                *p++ = c_encoding_table[((data[i] & 0x3) << 4) | ((int)(data[i + 1] & 0xF0) >> 4)];
                *p++ = c_encoding_table[((data[i + 1] & 0xF) << 2)];
            }
            *p++ = '=';
        }

        return ret;
    }

    bool Base64::decode(std::string_view input, std::vector<unsigned char>& result)
    {
        static constexpr unsigned char c_decoding_table[] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57,
            58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,
            7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
            25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
            37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64 
        };

        size_t in_len = input.size();
        if ((in_len % 4) != 0)
        {
            Vadon::Core::Logger::log_error("Base64 decoder error: input data size is not a multiple of 4!\n");
            return false;
        }

        size_t out_len = in_len / 4 * 3;
        if ((in_len >= 1) && (input[in_len - 1] == '='))
        {
            out_len--;
        }
        if ((in_len >= 2) && (input[in_len - 2] == '='))
        {
            out_len--;
        }

        result.reserve(result.size() + out_len);

        for (size_t i = 0, j = 0; i < in_len;) 
        {
            uint32_t a = input[i] == '=' ? 0 & i++ : c_decoding_table[static_cast<int>(input[i++])];
            uint32_t b = input[i] == '=' ? 0 & i++ : c_decoding_table[static_cast<int>(input[i++])];
            uint32_t c = input[i] == '=' ? 0 & i++ : c_decoding_table[static_cast<int>(input[i++])];
            uint32_t d = input[i] == '=' ? 0 & i++ : c_decoding_table[static_cast<int>(input[i++])];

            uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

            if (j < out_len)
            {
                result.push_back((triple >> 2 * 8) & 0xFF);
                ++j;
            }
            if (j < out_len)
            {
                result.push_back((triple >> 1 * 8) & 0xFF);
                ++j;
            }
            if (j < out_len)
            {
                result.push_back((triple >> 0 * 8) & 0xFF);
                ++j;
            }
        }

        return "";
    }
}