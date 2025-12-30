add_subdirectory(Project)
add_subdirectory(System)
add_subdirectory(TypeInfo)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Editor.hpp
    Editor.cpp
    Module.hpp
   )