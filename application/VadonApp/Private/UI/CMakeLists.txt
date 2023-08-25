add_subdirectory(Developer)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    Console.hpp
    Console.cpp
    UISystem.hpp
    UISystem.cpp
)