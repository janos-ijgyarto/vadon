target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Core.hpp
	)

target_precompile_headers(${VADON_CURRENT_TARGET} PRIVATE Core.hpp)