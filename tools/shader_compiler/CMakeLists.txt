set(VADON_SHADER_COMPILER "VadonShaderCompiler")

project(${VADON_SHADER_COMPILER}
        VERSION 0.0.1
        DESCRIPTION "Vadon shader compiler (with CMake)"
        LANGUAGES C CXX)

# NOTE: for tools, we want their binaries to end up in their own directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_executable(${VADON_SHADER_COMPILER} main.cpp)

target_compile_features(${VADON_SHADER_COMPILER} PRIVATE cxx_std_20)

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

if(MSVC)
  target_compile_options(${VADON_SHADER_COMPILER} PRIVATE /W4 /WX)
else()
  target_compile_options(${VADON_SHADER_COMPILER} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(VADON_TOOLS_EXPORT)
	export(TARGETS ${VADON_SHADER_COMPILER}
		   NAMESPACE VadonTools::
		   FILE "${VADON_TOOLS_EXPORT_FILE_DIR}/${VADON_SHADER_COMPILER}.cmake"
	)
endif()