target_sources(${VADONDEMO_CURRENT_TARGET}
    PRIVATE
        RenderSystem.hpp
        RenderSystem.cpp
   )

set(VADONDEMO_SHADER_LIST "")

vadon_compile_shader_to_embedded(VADONDEMO_SHADER_LIST ${VADONDEMO_SHADERS_ROOT_DIR}
 ${VADONDEMO_SHADER_OUTPUT_DIR} "CopyShader.hlsl" "VERTEX" "vs_main" "VadonDemo::Render::ShaderVS")

vadon_compile_shader_to_embedded(VADONDEMO_SHADER_LIST ${VADONDEMO_SHADERS_ROOT_DIR}
 ${VADONDEMO_SHADER_OUTPUT_DIR} "CopyShader.hlsl" "PIXEL" "ps_main" "VadonDemo::Render::ShaderPS")

# FIXME: proper name to distinguish from "root" target
add_custom_target("VadonDemoCopyShaders"
	DEPENDS ${VADONDEMO_SHADER_LIST}
)
set_target_properties("VadonDemoCopyShaders" PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

add_dependencies(${VADONDEMO_SHADERS_TARGET} "VadonDemoCopyShaders")