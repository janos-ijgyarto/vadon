#include <VadonApp/Private/PCH/VadonApp.hpp>
#include <VadonApp/Core/Application.hpp>

#include <VadonApp/Private/Core/Application.hpp>

namespace VadonApp::Core
{
    Application::Instance Application::create_instance()
    {
        return std::make_unique<VadonApp::Private::Core::Application>();
    }
}