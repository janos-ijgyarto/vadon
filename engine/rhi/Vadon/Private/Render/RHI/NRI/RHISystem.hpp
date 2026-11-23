#ifndef VADON_PRIVATE_RENDER_RHI_NRI_RHISYSTEM_HPP
#define VADON_PRIVATE_RENDER_RHI_NRI_RHISYSTEM_HPP
#include <Vadon/Private/Render/RHI/RHISystem.hpp>
#include <Vadon/Private/Render/RHI/NRI/Device/Device.hpp>
namespace Vadon::Private::Render::RHI::NRI
{
	class RHISystem : public Vadon::Private::Render::RHI::RHISystemBase
	{
	public:
		RHISystem(Vadon::Core::EngineCoreInterface& core, const DeviceDescription& description);

		bool initialize() override;
		void shutdown() override;

		Vadon::Render::RHI::Device& get_device() override { return m_device; }
	private:
		Device m_device;
	};
}
#endif