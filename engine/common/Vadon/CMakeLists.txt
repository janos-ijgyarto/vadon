add_subdirectory(Core)
add_subdirectory(ECS)
add_subdirectory(Math)
add_subdirectory(Private)
add_subdirectory(Scene)
add_subdirectory(Utilities)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	Common.hpp
   )