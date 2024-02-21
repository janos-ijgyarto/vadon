#ifndef VADON_UTILITIES_TYPE_TYPEINFO_HPP
#define VADON_UTILITIES_TYPE_TYPEINFO_HPP
#include <array>
#include <string_view>
namespace Vadon::Utilities
{
    template <std::size_t...INDICES>
    constexpr auto substring_as_array(std::string_view string, std::index_sequence<INDICES...>)
    {
        return std::array{ string[INDICES]... };
    }

    template <typename T>
    constexpr auto type_name_array()
    {
#if defined(__clang__)
        constexpr auto prefix = std::string_view{ "[T = " };
        constexpr auto suffix = std::string_view{ "]" };
        constexpr auto function = std::string_view{ __PRETTY_FUNCTION__ };
#elif defined(__GNUC__)
        constexpr auto prefix = std::string_view{ "with T = " };
        constexpr auto suffix = std::string_view{ "]" };
        constexpr auto function = std::string_view{ __PRETTY_FUNCTION__ };
#elif defined(_MSC_VER)
        constexpr auto prefix = std::string_view{ "type_name_array<" };
        constexpr auto suffix = std::string_view{ ">(void)" };
        constexpr auto function = std::string_view{ __FUNCSIG__ };
#else
# error Unsupported compiler
#endif

        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end = function.rfind(suffix);

        static_assert(start < end);

        constexpr auto name = function.substr(start, (end - start));
        return substring_as_array(name, std::make_index_sequence<name.size()>{});
    }

    template <typename T>
    struct TypeName
    {
        static inline constexpr auto value = type_name_array<T>();

        static constexpr std::string_view value_string() { return std::string_view{ value.data(), value.size() }; }

        static constexpr std::string_view trimmed()
        {
            constexpr auto prefix_array = std::array{
                std::string_view{"class "},
                std::string_view{"struct "}
            };

            constexpr std::string_view type_name = value_string();

            constexpr auto class_start = type_name.find(prefix_array[0]);
            if constexpr (class_start != std::string::npos)
            {
                return type_name.substr(class_start + prefix_array[0].size());
            }
            else 
            {
                constexpr auto struct_start = type_name.find(prefix_array[1]);
                if constexpr (struct_start != std::string::npos)
                {
                    return type_name.substr(struct_start + prefix_array[1].size());
                }
                else
                {
                    return type_name;
                }
            }
        }
    };
}
#endif