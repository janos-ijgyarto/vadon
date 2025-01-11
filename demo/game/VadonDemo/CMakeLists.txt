add_subdirectory(Core)
add_subdirectory(Model)
add_subdirectory(Platform)
add_subdirectory(Render)
add_subdirectory(UI)
add_subdirectory(View)

target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
    main.cpp
   )