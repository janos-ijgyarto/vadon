add_subdirectory(Main)
add_subdirectory(Project)
add_subdirectory(Scene)
add_subdirectory(Widgets)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    ViewSystem.hpp
    ViewSystem.cpp
   )