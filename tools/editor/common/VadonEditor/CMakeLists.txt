add_subdirectory(Network)
add_subdirectory(Simulator)

target_sources(${VADONEDITOR_COMMON}
    PRIVATE
    Common.hpp
)