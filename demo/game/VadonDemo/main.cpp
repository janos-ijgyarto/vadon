#include <VadonDemo/Core/GameCore.hpp>

#include <Vadon/Core/Environment.hpp>

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;

    VadonDemo::Core::GameCore game_core(engine_environment);
    return game_core.execute(argc, argv);
}