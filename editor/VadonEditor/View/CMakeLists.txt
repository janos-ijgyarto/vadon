add_subdirectory(Main)
add_subdirectory(Project)
add_subdirectory(Scene)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Module.hpp
    ViewSystem.hpp
    ViewSystem.cpp
   )