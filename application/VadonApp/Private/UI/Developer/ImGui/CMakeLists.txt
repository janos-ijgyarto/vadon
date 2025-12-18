include(Shader/Shader)
add_subdirectory(Fonts)

target_sources(${VADONAPP_CURRENT_TARGET}
    PRIVATE
    GUI.hpp
    GUI.cpp
    GUIElements.hpp
)

set(VADONAPP_IMGUI_SHADER_LIST "")

vadon_compile_shader_to_embedded(VADONAPP_IMGUI_SHADER_LIST ${VADONAPP_SHADERS_ROOT_DIR}
 ${VADONAPP_SHADER_OUTPUT_DIR} "GUIShader.hlsl" "VERTEX" "vs_main" "VadonApp::Private::UI::Developer::ImGUI::ShaderVS")

vadon_compile_shader_to_embedded(VADONAPP_IMGUI_SHADER_LIST ${VADONAPP_SHADERS_ROOT_DIR}
 ${VADONAPP_SHADER_OUTPUT_DIR} "GUIShader.hlsl" "PIXEL" "ps_main" "VadonApp::Private::UI::Developer::ImGUI::ShaderPS")

add_custom_target("VadonAppImGuiShaders"
	DEPENDS ${VADONAPP_IMGUI_SHADER_LIST}
)
set_target_properties("VadonAppImGuiShaders" PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")
add_dependencies("VadonAppImGuiShaders" VadonToolsBuild) # Make sure shader compiler is built first
add_dependencies(${VADONAPP_SHADERS_TARGET} "VadonAppImGuiShaders")