set(VADON_SHADER_COMPILER "ShaderCompiler")

project(${VADON_SHADER_COMPILER}
        VERSION 0.0.1
        DESCRIPTION "Vadon shader compiler (with CMake)"
        LANGUAGES C CXX)

# NOTE: for tools, we want their binaries to end up in their own directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_executable(${VADON_SHADER_COMPILER} main.cpp)
target_link_libraries(${VADON_SHADER_COMPILER} PRIVATE VadonBuildOptions)

set_target_properties(${VADONEDITOR_APPLICATION}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)

# Link to relevant libraries based on platform
if(WIN32)
	find_library(D3D_COMPILER_LIBRARY d3dcompiler)
	if(D3D_COMPILER_LIBRARY)
		target_link_libraries(${VADON_SHADER_COMPILER} PRIVATE ${D3D_COMPILER_LIBRARY})
		target_compile_definitions(${VADON_SHADER_COMPILER} PRIVATE VADON_D3D_COMPILER)
	endif()
	# TODO: link to other options if available (OGL, Vulkan, etc.)
endif()

if(UNIX AND NOT APPLE)
	# TODO: set up OpenGL/Vulkan for Linux!
endif()

if(VADON_TOOLS_EXPORT)
	export(TARGETS ${VADON_SHADER_COMPILER}
		FILE "${VADON_TOOLS_EXPORT_FILE_DIR}/ShaderCompiler.cmake"
		NAMESPACE VadonTools::
	)
endif()