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

vadon_generate_embedded_shader_source(${VADON_CURRENT_TARGET} Shader.hlsl Vadon::Private::Render::Canvas::Shader)