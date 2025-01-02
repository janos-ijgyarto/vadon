#ifndef VADON_RENDER_CANVAS_CONTEXT_HPP
#define VADON_RENDER_CANVAS_CONTEXT_HPP
#include <Vadon/Render/Canvas/Layer.hpp>
#include <Vadon/Render/Utilities/Rectangle.hpp>
#include <Vadon/Render/GraphicsAPI/RenderTarget/RenderTarget.hpp>
namespace Vadon::Render::Canvas
{
	struct Camera
	{
		Vadon::Render::Rectangle view_rectangle;
		float zoom = 1.0f;
	};

	struct Viewport
	{
		Vadon::Render::RTVHandle render_target;
		Vadon::Render::Viewport render_viewport;
	};

	struct RenderContext
	{
		Camera camera;
		std::vector<LayerHandle> layers;
		std::vector<Viewport> viewports;
	};
}
#endif