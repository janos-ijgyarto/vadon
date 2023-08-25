add_subdirectory(Dummy)
add_subdirectory(ImGui)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    GUI.hpp
    GUI.cpp
)