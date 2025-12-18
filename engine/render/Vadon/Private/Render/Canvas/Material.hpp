#ifndef VADON_PRIVATE_RENDER_CANVAS_MATERIAL_HPP
#define VADON_PRIVATE_RENDER_CANVAS_MATERIAL_HPP
#include <Vadon/Render/Canvas/Material.hpp>
namespace Vadon::Private::Render::Canvas
{
	using SDFParameters = Vadon::Render::Canvas::SDFParameters;
	using MaterialInfo = Vadon::Render::Canvas::MaterialInfo;

	using MaterialHandle = Vadon::Render::Canvas::MaterialHandle;

	struct MaterialData
	{
		MaterialInfo info;
		SDFParameters sdf;
		// TODO: other data?
	};

	using RenderState = Vadon::Render::Canvas::RenderState;

	using ColorOverride = Vadon::Render::Canvas::ColorOverride;
	using MaterialOverrideFlags = Vadon::Render::Canvas::MaterialOverrideFlags;
	using MaterialOverride = Vadon::Render::Canvas::MaterialOverride;
}
#endif