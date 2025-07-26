#ifndef VADON_RENDER_UTILITIES_EMBEDDEDSHADER_HPP
#define VADON_RENDER_UTILITIES_EMBEDDEDSHADER_HPP
namespace Vadon::Render
{
	struct EmbeddedShaderData
	{
		size_t shader_count;
		const char* const* shader_apis;
		const size_t* shader_data_sizes;
		const char* shader_data;

		size_t get_shader_index(const char* api_name) const
		{
			for (size_t i = 0; i < shader_count; ++i)
			{
				if (std::string_view(api_name) == std::string_view(shader_apis[i]))
				{
					return i;
				}
			}

			return size_t(-1);
		}

		size_t get_shader_data_offset(size_t shader_index) const
		{
			size_t offset = 0;
			for (size_t i = 0; i < shader_index; ++i)
			{
				offset += shader_data_sizes[i];
			}

			return offset;
		}
	};
}
#define VADON_GET_EMBEDDED_SHADER_DATA(shader_namespace) Vadon::Render::EmbeddedShaderData{\
	.shader_count = shader_namespace::c_shader_count,\
	.shader_apis = shader_namespace::c_shader_apis,\
	.shader_data_sizes = shader_namespace::c_shader_data_sizes,\
	.shader_data = shader_namespace::c_shader_data\
}
#endif