#ifndef VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_RESOURCE_HPP
#define VADON_PRIVATE_RENDER_GRAPHICSAPI_DUMMY_SHADER_RESOURCE_HPP
#include <Vadon/Render/GraphicsAPI/Shader/Resource.hpp>
namespace Vadon::Private::Render::Dummy
{
    using ShaderResourceType = Vadon::Render::ShaderResourceType;

    using Texture2DSRVInfo = Vadon::Render::Texture2DSRVInfo;

    using ShaderResourceInfo = Vadon::Render::ShaderResourceInfo;

    using ShaderResourceHandle = Vadon::Render::ShaderResourceHandle;
}
#endif