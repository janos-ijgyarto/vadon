add_subdirectory(Resource)
add_subdirectory(Scene)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    ModelSystem.hpp
    ModelSystem.cpp
    Module.hpp
   )