MESSAGE("Configuring Vadon Demo")

project(VadonDemo
        VERSION 0.0.1
        DESCRIPTION "Vadon implementation (with CMake)"
        LANGUAGES C CXX)

add_subdirectory(common)
add_subdirectory(editor)
add_subdirectory(game)