#include <VadonEditor/Core/Application.hpp>

int main(int argc, char* argv[])
{
    VadonEditor::Core::Application application(argc, argv);
    return application.exec();
}