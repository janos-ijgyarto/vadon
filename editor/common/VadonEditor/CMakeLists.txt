add_subdirectory(Core)
add_subdirectory(Scene)

target_sources(${VADONEDITOR_CURRENT_TARGET}
    PRIVATE
    VadonEditor.hpp
   )