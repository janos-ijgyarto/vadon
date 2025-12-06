add_subdirectory(Core)
add_subdirectory(Network)
add_subdirectory(Simulator)
add_subdirectory(UI)

# TODO: use explicit handling of .ui files instead of using AutoUIC!

target_sources(${VADONEDITOR_APPLICATION}
    PRIVATE
    main.cpp
)