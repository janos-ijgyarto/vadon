#ifndef VADONDEMO_RENDER_RESOURCE_HPP
#define VADONDEMO_RENDER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Resource/SRV.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <Vadon/Render/GraphicsAPI/Texture/Texture.hpp>
#include <Vadon/Model/Resource/File.hpp>
namespace Vadon::Foundation
{
	class TypeMetadataRegistry;
}
namespace VadonDemo::Render
{
	struct CanvasLayerDefinition : public Vadon::Model::Resource
	{
		VADON_DECLARE_MEMBER_UUID(priority, "58b85ba6-2561-4fdf-bab4-d5fb33871a78");
		VADON_DECLARE_MEMBER_UUID(view_agnostic, "f2664317-707c-4dda-8f0c-59d58ad5ade6");

		int priority = 0;
		bool view_agnostic = false;

		static void register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};
	
	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(CanvasLayerDefinition, CanvasLayerDefID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(CanvasLayerDefinition, CanvasLayerDefHandle);

	// TODO: implement resources that actually store texture data
	// Loading the resource means it's actually "ready" (i.e loaded as a GPU resource),
	// we don't keep the file data around
	struct TextureResource : public Vadon::Model::Resource
	{
		VADON_DECLARE_MEMBER_UUID(texture_file, "ba77ee5e-0fe4-49c4-9253-281d61e891a0");

		Vadon::Model::FileResourceID texture_file;

		Vadon::Render::TextureHandle texture;
		Vadon::Render::SRVHandle texture_srv;

		static void register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(TextureResource, TextureResourceID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(TextureResource, TextureResourceHandle);

	// TODO: implement proper shader resources!
	struct ShaderResource : public Vadon::Model::Resource
	{
		VADON_DECLARE_MEMBER_UUID(shader_file, "dd719674-79dc-45a5-8422-fdc850ffabb2");

		Vadon::Model::FileResourceID shader_file;

		Vadon::Render::ShaderHandle pixel_shader;

		static void register_resource(::Vadon::Foundation::TypeMetadataRegistry& metadata_registry);
	};

	VADON_MODEL_DECLARE_TYPED_RESOURCE_ID(ShaderResource, ShaderResourceID);
	VADON_MODEL_DECLARE_TYPED_RESOURCE_HANDLE(ShaderResource, ShaderResourceHandle);
}

VADON_REGISTER_TYPE_UUID(VadonDemo::Render::CanvasLayerDefinition, "6662a8a3-99c5-4e27-8f35-ee77e6130567");
VADON_REGISTER_TYPE_UUID(VadonDemo::Render::TextureResource, "abaa625c-dc1e-41ca-98dd-73f64cde6852");
VADON_REGISTER_TYPE_UUID(VadonDemo::Render::ShaderResource, "65a72547-c29b-4a5a-9e1d-ad36d02bea94");
#endif