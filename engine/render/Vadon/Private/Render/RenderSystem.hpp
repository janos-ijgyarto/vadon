#ifndef VADON_PRIVATE_RENDER_RENDERSYSTEM_HPP
#define VADON_PRIVATE_RENDER_RENDERSYSTEM_HPP
#include <Vadon/Render.hpp>
#include <Vadon/Private/Render/Frame/FrameSystem.hpp>
namespace Vadon::Core
{
	class EngineCoreInterface;
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
	private:
		Vadon::Core::EngineCoreInterface& m_core;
		FrameSystem m_frame_system;
	};
}
#endif