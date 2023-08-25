add_subdirectory(Buffer)
add_subdirectory(Pipeline)
add_subdirectory(RenderTarget)
add_subdirectory(Shader)
add_subdirectory(Texture)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	GraphicsAPI.hpp
	GraphicsModule.hpp
	)