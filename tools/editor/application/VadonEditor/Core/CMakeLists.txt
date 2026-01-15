add_subdirectory(Project)

target_sources(${VADONEDITOR_APPLICATION}
    PRIVATE
    Application.hpp
    Application.cpp
    CommandLine.hpp
    Logger.hpp
    Logger.cpp
)