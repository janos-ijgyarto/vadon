#ifndef VADON_PRIVATE_RENDER_RHI_NULL_RHISYSTEM_HPP
#define VADON_PRIVATE_RENDER_RHI_NULL_RHISYSTEM_HPP
#include <Vadon/Private/Render/RHI/RHISystem.hpp>
#include <Vadon/Private/Render/RHI/Null/Device/Device.hpp>
namespace Vadon::Private::Render::RHI::Null
{
	class RHISystem : public Vadon::Private::Render::RHI::RHISystemBase
	{
	public:
		RHISystem(Vadon::Core::EngineCoreInterface& core);

		bool initialize() override;
		void shutdown() override;

		Vadon::Render::RHI::Device& get_device() override { return m_device; }
	private:
		Device m_device;
	};
}
#endif