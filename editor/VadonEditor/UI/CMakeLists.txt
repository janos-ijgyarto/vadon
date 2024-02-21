add_subdirectory(Developer)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    Module.hpp
    UISystem.hpp
    UISystem.cpp
   )