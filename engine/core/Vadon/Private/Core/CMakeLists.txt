add_subdirectory(File)
add_subdirectory(Task)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
        Core.hpp
        Core.cpp
   )