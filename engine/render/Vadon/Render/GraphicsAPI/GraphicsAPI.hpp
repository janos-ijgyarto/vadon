#ifndef VADON_RENDER_GRAPHICSAPI_GRAPHICSAPI_HPP
#define VADON_RENDER_GRAPHICSAPI_GRAPHICSAPI_HPP
#include <Vadon/Render/GraphicsAPI/GraphicsModule.hpp>
#include <Vadon/Utilities/Data/DataUtilities.hpp>
namespace Vadon::Render
{
	// FIXME: implement other types?
	enum class DrawCommandType
	{
		DEFAULT,
		INDEXED,
		INSTANCED,
		INDEXED_INSTANCED
	};

	struct DrawCommand
	{
		DrawCommandType type = DrawCommandType::DEFAULT;
		
		Vadon::Utilities::DataRange vertices;
		Vadon::Utilities::DataRange indices;
		Vadon::Utilities::DataRange instances;
	};

	class GraphicsAPI : public GraphicsSystem<GraphicsAPI>
	{
	public:
		virtual ~GraphicsAPI() {}

		virtual void draw(const DrawCommand& command) = 0;
	protected:
		GraphicsAPI(Core::EngineCoreInterface& core) 
			: EngineSystem(core) 
		{
		}
	};
}
#endif