add_subdirectory(Object)
add_subdirectory(Task)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
        Core.hpp
        Core.cpp
   )