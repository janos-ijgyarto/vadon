add_subdirectory(Frame)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	RenderSystem.hpp
	RenderSystem.cpp
	)