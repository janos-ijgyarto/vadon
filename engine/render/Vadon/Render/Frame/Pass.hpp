#ifndef VADON_RENDER_FRAME_PASS_HPP
#define VADON_RENDER_FRAME_PASS_HPP
#include <Vadon/Render/GraphicsAPI/Pipeline/Pipeline.hpp>
#include <Vadon/Render/GraphicsAPI/Shader/Shader.hpp>
#include <functional>
namespace Vadon::Render
{
	using RenderPassFunction = std::function<void()>; // FIXME: should this have any overlap with the task system?
	using RenderPassValidator = std::function<bool()>;

	struct PassResource
	{
		std::string name;
		std::string out_name;
		// TODO: anything else?

		bool operator==(const PassResource& rhs) const { return (name == rhs.name); }
	};

	struct RenderPass
	{
		static constexpr bool default_validator_function() { return true; }

		std::string name;

		std::vector<PassResource> shader_resources;
		std::vector<PassResource> targets;

		std::vector<ShaderHandle> shaders;

		PipelineState pipeline_state;

		// TODO: anything else?

		RenderPassFunction render_function;
		RenderPassValidator validator_function = &default_validator_function;
	};
}
#endif