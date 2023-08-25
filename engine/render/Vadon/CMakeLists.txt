add_subdirectory(Render)
add_subdirectory(Private)

target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	Render.hpp
   )