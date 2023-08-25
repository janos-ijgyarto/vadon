target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Common.hpp
	)

target_precompile_headers(${VADON_CURRENT_TARGET} INTERFACE Common.hpp)