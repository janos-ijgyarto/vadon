add_subdirectory(Canvas)
add_subdirectory(Frame)
add_subdirectory(Utilities)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	RenderSystem.hpp
	RenderSystem.cpp
	)