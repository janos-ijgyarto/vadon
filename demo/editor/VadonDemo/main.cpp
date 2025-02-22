#include <VadonDemo/Core/Editor.hpp>

#include <Vadon/Core/Environment.hpp>

int main(int argc, char* argv[])
{
    Vadon::Core::EngineEnvironment engine_environment;
    Vadon::Core::EngineEnvironment::initialize(engine_environment);

    VadonDemo::Core::Editor editor(engine_environment);
    return editor.execute(argc, argv);
}