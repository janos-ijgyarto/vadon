add_subdirectory(Core)
add_subdirectory(Private)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	Core.hpp
   )