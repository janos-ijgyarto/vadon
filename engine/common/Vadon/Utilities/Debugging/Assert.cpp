#include <Vadon/Utilities/Debugging/Assert.hpp>

#include <Vadon/Core/Logger.hpp>

namespace Vadon::Utilities
{
    void do_assert(const char* expression_string, std::string_view message, std::source_location location)
    {
        Vadon::Core::Logger::log_error(std::format("Assertion failed!\n{}\n\"{}\"\nFile: {}\nFunction: {}\nLine: {}\nColumn: {}",
            expression_string, message, location.file_name(), location.function_name(), location.line(), location.column()));

        VADON_ASSERT_BREAKPOINT();
    }

    void do_error(std::string_view message, std::source_location location)
    {
        Vadon::Core::Logger::log_error(std::format("Error!\n\"{}\"\nFile: {}\nFunction: {}\nLine: {}\nColumn: {}",
            message, location.file_name(), location.function_name(), location.line(), location.column()));

        VADON_ASSERT_BREAKPOINT();
    }
}