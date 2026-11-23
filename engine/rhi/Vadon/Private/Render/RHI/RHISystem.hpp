#ifndef VADON_PRIVATE_RENDER_RHI_RHISYSTEM_HPP
#define VADON_PRIVATE_RENDER_RHI_RHISYSTEM_HPP
#include <Vadon/RHI.hpp>
#include <Vadon/Render/RHI/RHISystem.hpp>
#include <memory>
namespace Vadon::Core
{
	class EngineEnvironment;
}
namespace Vadon::Private::Render::RHI
{
	class RHISystemBase : public Vadon::Render::RHI::RHISystem
	{
	public:
		using Implementation = std::unique_ptr<RHISystemBase>;

		virtual bool initialize() = 0;
		virtual void shutdown() = 0;

		static VADONRHI_API void init_engine_environment(Vadon::Core::EngineEnvironment& environment);

		static VADONRHI_API Implementation get_rhi(Vadon::Core::EngineCoreInterface& core);
		static VADONRHI_API Implementation get_null_rhi(Vadon::Core::EngineCoreInterface& core);
	protected:
		RHISystemBase(Vadon::Core::EngineCoreInterface& core) : Vadon::Render::RHI::RHISystem(core) {}
	};
}
#endif