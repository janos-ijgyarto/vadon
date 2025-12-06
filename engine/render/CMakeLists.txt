MESSAGE("Configuring Vadon Render")

# Add shader compiler
include(${VADON_TOOLS_EXPORT_FILE_DIR}/ShaderCompiler.cmake)

set(VADONRENDER_SHADERS_TARGET "VadonRenderShaders")
add_custom_target(${VADONRENDER_SHADERS_TARGET})

# Make dependent on the tools being built
add_dependencies(${VADONRENDER_SHADERS_TARGET} VadonToolsBuild)
set_target_properties(${VADONRENDER_SHADERS_TARGET} PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

set(VADONRENDER_SHADERS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VADONRENDER_CODEGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/codegen")
set(VADONRENDER_SHADER_OUTPUT_DIR "${VADONRENDER_CODEGEN_DIR}/shaders")

VadonCreateEngineTarget("VadonRender" "Render" "VADONRENDER_EXPORTS")

# Make target depend on shader codegen
add_dependencies(${VADON_CURRENT_TARGET} ${VADONRENDER_SHADERS_TARGET})

# Add the shader codegen to include paths
target_include_directories(${VADON_CURRENT_TARGET} PRIVATE ${VADONRENDER_SHADER_OUTPUT_DIR})

set(VADON_RENDER_LIBRARIES Vadon::Common freetype)
target_link_libraries(${VADON_CURRENT_TARGET} PUBLIC ${VADON_RENDER_LIBRARIES})