target_sources(${VADON_CURRENT_TARGET}
    PRIVATE
	GraphicsAPI.hpp
   )

target_precompile_headers(${VADON_CURRENT_TARGET} PRIVATE GraphicsAPI.hpp)