add_subdirectory(Canvas)
add_subdirectory(Frame)
add_subdirectory(GraphicsAPI)
add_subdirectory(Utilities)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	RenderModule.hpp
	)