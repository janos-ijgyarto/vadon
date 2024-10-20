add_subdirectory(Developer)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    Configuration.hpp
    Console.hpp
    Module.hpp
    UISystem.hpp
)