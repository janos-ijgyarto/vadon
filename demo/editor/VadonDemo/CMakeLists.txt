add_subdirectory(Core)
add_subdirectory(Network)
add_subdirectory(Platform)
add_subdirectory(Render)
add_subdirectory(UI)
add_subdirectory(View)

target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
    VadonDemoEditor.cpp
)