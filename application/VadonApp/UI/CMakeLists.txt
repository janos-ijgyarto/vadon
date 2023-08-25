add_subdirectory(Developer)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    Configuration.hpp
    Console.hpp
    UISystem.hpp
)