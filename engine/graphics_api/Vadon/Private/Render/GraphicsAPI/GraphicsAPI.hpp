#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_GRAPHICSAPI_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_GRAPHICSAPI_HPP
#include <Vadon/GraphicsAPI.hpp>
#include <Vadon/Render/GraphicsAPI/GraphicsAPI.hpp>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace Vadon::Private::Render
{
	class GraphicsAPIBase : public Vadon::Render::GraphicsAPI
	{
	public:
		using Implementation = std::unique_ptr<GraphicsAPIBase>;

		virtual bool initialize() = 0;
		virtual void update() = 0;
		virtual void shutdown() = 0;

		static VADONGRAPHICSAPI_API void init_engine_environment(Vadon::Core::EngineEnvironment& environment);

		static VADONGRAPHICSAPI_API Implementation get_graphics_api(Vadon::Core::EngineCoreInterface& core);
		static VADONGRAPHICSAPI_API Implementation get_null_graphics_api(Vadon::Core::EngineCoreInterface& core);
	protected:
		GraphicsAPIBase(Vadon::Core::EngineCoreInterface& core) : Vadon::Render::GraphicsAPI(core) {}
	};
}
#endif