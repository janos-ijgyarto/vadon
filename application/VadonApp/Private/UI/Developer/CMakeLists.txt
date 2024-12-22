add_subdirectory(ImGui)
add_subdirectory(Null)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    GUI.hpp
    GUI.cpp
)