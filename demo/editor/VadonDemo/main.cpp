#include <VadonDemo/Core/Editor.hpp>

#include <Vadon/Core/Environment.hpp>
#include <Vadon/Core/Logger.hpp>

#include <iostream>
#include <format>

namespace
{
    void log_unhandled_exception(std::exception& exception)
    {
        if (Vadon::Core::EngineEnvironment::is_initialized() == true)
        {
            Vadon::Core::EngineEnvironment::get_logger().log_error(std::format("Unhandled exception: \"{}\"", exception.what()));
            VADON_ASSERT_BREAKPOINT();
        }
        else
        {
            std::cerr << "Unhandled exception: \"" << exception.what() << "\"" << std::endl;
        }
    }

    int main_internal(int argc, char* argv[], Vadon::Core::EngineEnvironment& environment)
    {
        Vadon::Core::EngineEnvironment::initialize(environment);

        VadonDemo::Core::Editor editor(environment);

        return editor.execute(argc, argv);
    }
}

int main(int argc, char* argv[])
{
    int return_value = EXIT_SUCCESS;
    Vadon::Core::EngineEnvironment engine_environment;
    try
    {
        return_value = main_internal(argc, argv, engine_environment);
    }
    catch(std::exception& exception)
    {
        log_unhandled_exception(exception);
        return EXIT_FAILURE;
    }

    return return_value;
}