#ifndef VADON_RENDER_GRAPHICSAPI_DATA_FORMAT_HPP
#define VADON_RENDER_GRAPHICSAPI_DATA_FORMAT_HPP
namespace Vadon::Render
{
	// FIXME: better naming convention?
	enum class GraphicsAPIDataFormat
	{
		UNKNOWN,
		FLOAT4_32,
		FLOAT2_32,
		UNORM4_8,
		FLOAT_32,
		UINT_32,
		UINT_16,
		BC1_UNORM
	};

	// FIXME: better naming convention?
	enum class GraphicsAPIComparisonFunction
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};
}
#endif