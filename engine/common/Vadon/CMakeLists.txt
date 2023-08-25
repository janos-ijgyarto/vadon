add_subdirectory(Core)
add_subdirectory(Private)
add_subdirectory(Utilities)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	Common.hpp
   )