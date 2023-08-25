target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Render.hpp
	)

target_precompile_headers(${VADON_CURRENT_TARGET} PRIVATE Render.hpp)