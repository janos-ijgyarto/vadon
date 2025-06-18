#ifndef VADONDEMO_RENDER_RESOURCE_HPP
#define VADONDEMO_RENDER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Scene/Resource/Resource.hpp>
namespace VadonDemo::Render
{
	struct CanvasLayerDefinition : public Vadon::Scene::Resource
	{
		int priority = 0;
		bool view_agnostic = false;

		static void register_resource();
	};
	
	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(CanvasLayerDefinition, CanvasLayerDefID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(CanvasLayerDefinition, CanvasLayerDefHandle);

	// TODO: implement generic File resource
	// - Used as foundation for "this is a file we want to load"
	// - Base resource loads file as-is, delegates processing to subclasses

	// TODO: implement resources that actually store texture data
	// Loading the resource means it's actually "ready" (i.e loaded as a GPU resource),
	// we don't keep the file data around
	struct TextureResource : public Vadon::Scene::Resource
	{
		std::string file_path;

		Vadon::Render::TextureHandle texture;
		Vadon::Render::SRVHandle texture_srv;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(TextureResource, TextureResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(TextureResource, TextureResourceHandle);

	// TODO: implement proper shader resources!
	struct ShaderResource : public Vadon::Scene::Resource
	{
		std::string shader_path;

		Vadon::Render::ShaderHandle pixel_shader;

		static void register_resource();
	};

	VADON_SCENE_DECLARE_TYPED_RESOURCE_ID(ShaderResource, ShaderResourceID);
	VADON_SCENE_DECLARE_TYPED_RESOURCE_HANDLE(ShaderResource, ShaderResourceHandle);
}
#endif