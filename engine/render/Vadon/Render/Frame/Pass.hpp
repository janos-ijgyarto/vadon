#ifndef VADON_RENDER_FRAME_PASS_HPP
#define VADON_RENDER_FRAME_PASS_HPP
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <functional>
namespace Vadon::Render
{
	using RenderPassExecution = std::function<void()>; // FIXME: should this have any overlap with the task system?

	struct PassResource
	{
		std::string name;
		std::string out_name;
		// TODO: anything else?

		bool operator==(const PassResource& rhs) const { return (name == rhs.name); }
	};

	struct RenderPass
	{
		std::string name;

		// TODO: set the slots for the resources and targets?
		std::vector<PassResource> resources;
		std::vector<PassResource> targets;

		// TODO: anything else?

		RenderPassExecution execution;
	};
}
#endif