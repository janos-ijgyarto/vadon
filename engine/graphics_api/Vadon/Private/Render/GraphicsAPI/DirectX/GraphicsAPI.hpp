#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_GRAPHICSAPI_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DIRECTX_GRAPHICSAPI_HPP
#include <Vadon/Private/Render/GraphicsAPI/GraphicsAPI.hpp>
#include <Vadon/Private/Render/GraphicsAPI/DirectX/Defines.hpp>
namespace Vadon::Private::Render::DirectX
{
	struct D3DInfo
	{
		D3D_FEATURE_LEVEL m_feature_level;
	};

	class RenderTargetSystem;
	class ShaderSystem;

	class GraphicsAPI final : public Render::GraphicsAPIBase
	{
	public:
		using Device = ID3D11Device;
		using DeviceContext = ID3D11DeviceContext;

		GraphicsAPI(Vadon::Core::EngineCoreInterface& core);
		~GraphicsAPI();

		const D3DInfo& get_d3d_info() const { return m_d3d_info; }

		void draw(const Vadon::Render::DrawCommand& command) override;

		bool initialize() override;
		void update() override;
		void shutdown() override;

		IDXGIFactory6* get_dxgi_factory() const { return m_dxgi_factory.Get(); }
		Device* get_device() const { return m_device.Get(); }
		DeviceContext* get_device_context() const { return m_device_context.Get(); }

		RenderTargetSystem& get_directx_rt_system();
		ShaderSystem& get_directx_shader_system();
	private:
		D3DInfo m_d3d_info;

		ComPtr<IDXGIFactory6> m_dxgi_factory;
		ComPtr<IDXGIAdapter4> m_dxgi_adapter;

		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_device_context;
#ifndef NDEBUG
		ComPtr<ID3D11Debug> m_d3d_debug;
		ComPtr<ID3D11InfoQueue> m_info_queue;
#endif

		struct Internal;
		std::unique_ptr<Internal> m_internal;
	};
}
#endif