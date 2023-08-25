add_subdirectory(Core)
add_subdirectory(Platform)
add_subdirectory(Render)
add_subdirectory(UI)

target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
    main.cpp
   )