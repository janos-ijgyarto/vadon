add_subdirectory(System)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Editor.hpp
    Editor.cpp
    Module.hpp
   )