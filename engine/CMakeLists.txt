project(VadonMain
        VERSION 0.0.1
        DESCRIPTION "Vadon implementation (with CMake)"
        LANGUAGES C CXX)

add_subdirectory(third_party)
add_subdirectory(common)
add_subdirectory(render)
add_subdirectory(graphics_api)
add_subdirectory(core)