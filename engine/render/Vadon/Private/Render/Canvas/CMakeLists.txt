include(Shader/Shader)

target_sources(${VADON_CURRENT_TARGET}
	PRIVATE
	Batch.hpp
	CanvasSystem.hpp
	CanvasSystem.cpp
	Context.hpp
	Item.hpp
	Layer.hpp
	Material.hpp
	Primitive.hpp
	Transform.hpp
	)

set(VADON_RENDER_CANVAS_SHADER_LIST "")

vadon_compile_shader_to_embedded(VADON_RENDER_CANVAS_SHADER_LIST ${VADONRENDER_SHADERS_ROOT_DIR}
 ${VADONRENDER_SHADER_OUTPUT_DIR} "Shader.hlsl" "VERTEX" "vs_main" "Vadon::Private::Render::Canvas::ShaderVS")

vadon_compile_shader_to_embedded(VADON_RENDER_CANVAS_SHADER_LIST ${VADONRENDER_SHADERS_ROOT_DIR}
 ${VADONRENDER_SHADER_OUTPUT_DIR} "Shader.hlsl" "PIXEL" "ps_main" "Vadon::Private::Render::Canvas::ShaderPS")

add_custom_target("VadonRenderCanvasShaders"
	DEPENDS ${VADON_RENDER_CANVAS_SHADER_LIST}
)
set_target_properties("VadonRenderCanvasShaders" PROPERTIES FOLDER "${VADON_PARENT_FOLDER}${PROJECT_NAME}/Codegen")

add_dependencies(${VADONRENDER_SHADERS_TARGET} "VadonRenderCanvasShaders")
add_dependencies("VadonRenderCanvasShaders" VadonToolsBuild) # Make sure shader compiler is built first!