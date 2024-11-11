add_subdirectory(Fonts)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    GUI.hpp
    GUI.cpp
    GUIElements.hpp
)