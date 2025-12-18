#ifndef VADON_RENDER_CANVAS_MATERIAL_HPP
#define VADON_RENDER_CANVAS_MATERIAL_HPP
#include <Vadon/Math/Color.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Handle.hpp>
#include <Vadon/Utilities/Enum/EnumClassBitFlag.hpp>
namespace Vadon::Render::Canvas
{
	struct MaterialInfo
	{
		std::string name;
	};

	struct SDFParameters
	{
		enum class Flags
		{
			NONE = 0,
			ENABLED = 1 << 0
		};

		Flags flags = Flags::NONE;
		float edge_distance = 0.1f;
		float edge_width_factor = 0.7f;
	};

	// FIXME: should this use a dense pool for faster iteration?
	// The commands will probably be more important
	VADON_DECLARE_TYPED_POOL_HANDLE(Material, MaterialHandle);

	// FIXME: this is a barebones implementation, need to extend!
	struct RenderState
	{
		bool alpha_blend = false;
		// TODO: other params?

		bool operator==(const RenderState& other) const
		{
			return alpha_blend == other.alpha_blend;
		}
	};

	struct ColorOverride
	{
		Vadon::Math::ColorRGBA color = Vadon::Math::Color_White;
	};

	enum class MaterialOverrideFlags
	{
		NONE = 0,
		COLOR = 1 << 0,
		DEFAULT = NONE
	};

	struct MaterialOverride
	{
		ColorOverride color_override;
		MaterialOverrideFlags flags = MaterialOverrideFlags::DEFAULT;
	};
}
namespace Vadon::Utilities
{
	template<>
	struct EnableEnumBitwiseOperators<Vadon::Render::Canvas::MaterialOverrideFlags> : public std::true_type
	{

	};
}
#endif