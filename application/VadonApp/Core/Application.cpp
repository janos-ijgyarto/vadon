#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Private/Core/Application.hpp>

#include <Vadon/Core/Environment.hpp>

namespace VadonApp::Core
{
    void Application::init_application_environment(Vadon::Core::EngineEnvironment& environment)
    {
        // Initialize from both here and for the engine
        Vadon::Core::init_engine_environment(environment);
        Vadon::Core::EngineEnvironment::initialize(environment);
    }

    Application::Instance Application::create_instance()
    {
        return std::make_unique<VadonApp::Private::Core::Application>();
    }
}