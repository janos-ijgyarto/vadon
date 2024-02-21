add_subdirectory(Object)
add_subdirectory(System)
add_subdirectory(Task)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
        Configuration.hpp
        CoreInterface.hpp
        CoreModule.hpp
        Logger.hpp
   )