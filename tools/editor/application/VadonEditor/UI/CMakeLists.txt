add_subdirectory(Project)
add_subdirectory(Utilities)

target_sources(${VADONEDITOR_APPLICATION}
    PRIVATE
    MainWindow.hpp
    MainWindow.cpp
    UISystem.hpp
    UISystem.cpp
)