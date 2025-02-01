add_subdirectory(Model)
add_subdirectory(UI)
add_subdirectory(View)

target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
    VadonDemoCommon.hpp)