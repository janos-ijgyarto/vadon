#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Private/Core/Application.hpp>

#include <Vadon/Core/Core.hpp>
#include <Vadon/Core/Environment.hpp>

namespace VadonApp::Core
{
    void Application::init_application_environment(Vadon::Core::EngineEnvironment& environment)
    {
        // Initialize from both here and for the engine
        Vadon::Core::init_engine_environment(environment);
        Vadon::Core::EngineEnvironment::initialize(environment);
    }

    Application::Instance Application::create_instance(Vadon::Core::EngineCoreInterface& engine_core)
    {
        return std::make_unique<VadonApp::Private::Core::Application>(engine_core);
    }

    bool Application::has_command_line_arg(std::string_view name) const
    {
        auto arg_it = m_command_line_args.find(name.data());
        if (arg_it == m_command_line_args.end())
        {
            return false;
        }

        return true;
    }

    std::string Application::get_command_line_arg(std::string_view name) const
    {
        auto arg_it = m_command_line_args.find(name.data());
        if (arg_it == m_command_line_args.end())
        {
            return std::string();
        }

        return arg_it->second;
    }

    void Application::parse_command_line(int argc, char* argv[])
    {
        m_program_name = argv[0];

        for (int32_t current_arg_index = 1; current_arg_index < argc; ++current_arg_index)
        {
            parse_command_line_argument(argv[current_arg_index]);
        }
    }

    void Application::parse_command_line_argument(const char* argument_ptr)
    {
        const std::string argument_string(argument_ptr);

        const size_t equals_char_offset = argument_string.find('=');

        // TODO: check that the arg isn't just whitespace

        if (equals_char_offset == std::string::npos)
        {
            // No equals char found, assume it's a command
            m_command_line_args.emplace(argument_string, "");
            return;
        }

        // Emplace the argument name and value
        const std::string arg_name = argument_string.substr(0, equals_char_offset);
        const std::string arg_value = argument_string.substr(equals_char_offset + 1);

        m_command_line_args.emplace(arg_name, arg_value);
    }
}