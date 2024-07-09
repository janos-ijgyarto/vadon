#ifndef VADON_PRIVATE_RENDER_RENDERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_RENDERSYSTEM_HPP
#include <Vadon/Render.hpp>
#include <Vadon/Private/Render/Canvas/CanvasSystem.hpp>
#include <Vadon/Private/Render/Frame/FrameSystem.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
	struct EngineEnvironment;
}
namespace Vadon::Private::Render
{
	class RenderSystem
	{
	public:
		VADONRENDER_API RenderSystem(Vadon::Core::EngineCoreInterface& core);
		VADONRENDER_API ~RenderSystem();

		bool VADONRENDER_API initialize();
		void VADONRENDER_API update();
		void VADONRENDER_API shutdown();

		static VADONRENDER_API void init_engine_environment(Vadon::Core::EngineEnvironment& environment);
	private:
		Vadon::Core::EngineCoreInterface& m_core;
		Canvas::CanvasSystem m_canvas_system;
		FrameSystem m_frame_system;
	};
}
#endif