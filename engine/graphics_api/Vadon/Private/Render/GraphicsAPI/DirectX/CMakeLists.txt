add_subdirectory(Buffer)
add_subdirectory(Pipeline)
add_subdirectory(RenderTarget)
add_subdirectory(Resource)
add_subdirectory(Shader)
add_subdirectory(Texture)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	D3DCommon.hpp
	Defines.hpp
	GraphicsAPI.hpp
	GraphicsAPI.cpp
	)

# Add the preprocessor definition for DirectX
target_compile_definitions(${VADON_CURRENT_TARGET} PRIVATE VADON_GRAPHICS_API_DIRECTX)

# Link DirectX-related libraries
target_link_libraries(${VADON_CURRENT_TARGET} PRIVATE d3d11 dxgi dxguid d3dcompiler)