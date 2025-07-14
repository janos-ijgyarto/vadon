#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADER_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_SHADER_SHADER_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/D3DCommon.hpp>
namespace Vadon::Private::Render::DirectX
{
	using ShaderType = Vadon::Render::ShaderType;
	using ShaderDefine = Vadon::Render::ShaderDefine;

	using ShaderInfo = Vadon::Render::ShaderInfo;

	using D3DVertexShader = ComPtr<ID3D11VertexShader>;
	using D3DPixelShader = ComPtr<ID3D11PixelShader>;
	// TODO: geometry shader?
	using D3DBlob = ComPtr<ID3DBlob>;

	struct Shader
	{
		ShaderInfo info;

		// FIXME: only one of these will ever be active. Use variant perhaps?
		D3DVertexShader d3d_vertex_shader;
		D3DPixelShader d3d_pixel_shader;
		// TODO: geometry shader?
	};

	using ShaderHandle = Vadon::Render::ShaderHandle;

	using VertexElementType = Vadon::Render::VertexElementType;

	using VertexLayoutElement = Vadon::Render::VertexLayoutElement;

	using VertexLayoutInfo = Vadon::Render::VertexLayoutInfo;

	using D3DInputLayout = ComPtr<ID3D11InputLayout>;
	struct VertexLayout
	{
		D3DInputLayout d3d_input_layout;
	};

	using VertexLayoutHandle = Vadon::Render::VertexLayoutHandle;
}
#endif