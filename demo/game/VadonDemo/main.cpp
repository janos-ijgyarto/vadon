#include <VadonDemo/Core/GameCore.hpp>

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
        }
        else
        {
            std::cerr << "Unhandled exception: \"" << exception.what() << "\"" << std::endl;
        }
    }

    int main_internal(int argc, char* argv[])
    {
        Vadon::Core::EngineEnvironment engine_environment;

        VadonDemo::Core::GameCore game_core(engine_environment);
        return game_core.execute(argc, argv);
    }
}

int main(int argc, char* argv[])
{
    int return_value = EXIT_SUCCESS;
    try
    {
        return_value = main_internal(argc, argv);
    }
    catch (std::exception& exception)
    {
        log_unhandled_exception(exception);
        return EXIT_FAILURE;
    }

    return return_value;
}