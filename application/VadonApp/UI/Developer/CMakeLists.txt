add_subdirectory(Widgets)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    Configuration.hpp
    GUI.hpp
    GUIElements.hpp
    IconsFontAwesome7.h
)