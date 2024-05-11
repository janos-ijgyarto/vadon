#ifndef VADON_RENDER_GRAPHICSAPI_SHADER_SHADER_HPP
#define VADON_RENDER_GRAPHICSAPI_SHADER_SHADER_HPP
#include <Vadon/Render/GraphicsAPI/Defines.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <string>
namespace Vadon::Render
{
	enum class ShaderType
	{
		VERTEX,
		GEOMETRY,
		PIXEL,
		COMPUTE,
		TYPE_COUNT
	};

	// FIXME: create something that is compatible across APIs, for now we'll follow the bare minimum to use D3D
	struct ShaderDefine
	{
		std::string name;
		std::string value;
	};

	struct ShaderInfo
	{
		ShaderType type = ShaderType::VERTEX;
		std::string source;
		std::string name;
		std::string entrypoint;
		std::vector<ShaderDefine> defines;
	};

	VADON_DECLARE_TYPED_POOL_HANDLE(Shader, ShaderHandle);

	enum class VertexElementType
	{
		PER_VERTEX
	};

	struct VertexLayoutElement
	{
		std::string name;
		GraphicsAPIDataFormat format;
		VertexElementType type;
	};

	using VertexLayoutInfo = std::vector<VertexLayoutElement>;

	VADON_DECLARE_TYPED_POOL_HANDLE(VertexLayout, VertexLayoutHandle);

	// FIXME: more appropriate name?
	struct ShaderObject
	{
		ShaderHandle vertex_shader;
		ShaderHandle pixel_shader;

		VertexLayoutHandle vertex_layout;
	};
}
#endif