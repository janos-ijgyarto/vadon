#include <Vadon/Private/Render/RHI/NRI/Device/Device.hpp>

#include <Vadon/Private/Render/RHI/NRI/Common.hpp>
#include <Vadon/Private/Render/RHI/NRI/Device/Allocator.hpp>
#include <Vadon/Private/Render/RHI/NRI/Resource/CommandList.hpp>

#include <Vadon/Utilities/Container/Map/ConstexprMap.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>

#include <Vadon/Utilities/Enum/EnumBitFlagMapping.hpp>

#include <nri/NRI.h>

#include <nri/Extensions/NRIDescriptorHeap.h>
#include <nri/Extensions/NRIHelper.h>
#include <nri/Extensions/NRIStreamer.h>
#include <nri/Extensions/NRISwapChain.h>

#include <vector>

namespace
{
	constexpr auto c_nri_graphics_api_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::GraphicsAPI, nri::GraphicsAPI>({
		{ Vadon::Render::RHI::GraphicsAPI::NULL_API, nri::GraphicsAPI::NONE },
		{ Vadon::Render::RHI::GraphicsAPI::D3D12, nri::GraphicsAPI::D3D12 },
		{ Vadon::Render::RHI::GraphicsAPI::VULKAN, nri::GraphicsAPI::VK }
		});

	constexpr auto c_nri_buffer_usage_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::BufferUsage, nri::BufferUsageBits>({
		{ Vadon::Render::RHI::BufferUsage::SHADER_RESOURCE, nri::BufferUsageBits::SHADER_RESOURCE },
		{ Vadon::Render::RHI::BufferUsage::READ_WRITE, nri::BufferUsageBits::SHADER_RESOURCE_STORAGE },
		{ Vadon::Render::RHI::BufferUsage::VERTEX, nri::BufferUsageBits::VERTEX },
		{ Vadon::Render::RHI::BufferUsage::INDEX, nri::BufferUsageBits::INDEX },
		{ Vadon::Render::RHI::BufferUsage::CONSTANT, nri::BufferUsageBits::CONSTANT }
		});

	constexpr auto c_nri_queue_type_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::QueueType, nri::QueueType>({
		{ Vadon::Render::RHI::QueueType::GRAPHICS, nri::QueueType::GRAPHICS },
		{ Vadon::Render::RHI::QueueType::COMPUTE, nri::QueueType::COMPUTE },
		{ Vadon::Render::RHI::QueueType::COPY, nri::QueueType::COPY }
		});

	constexpr auto c_nri_texture_type_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::TextureType, nri::TextureType>({
		{ Vadon::Render::RHI::TextureType::TEXTURE_1D, nri::TextureType::TEXTURE_1D },
		{ Vadon::Render::RHI::TextureType::TEXTURE_2D, nri::TextureType::TEXTURE_2D },
		{ Vadon::Render::RHI::TextureType::TEXTURE_3D, nri::TextureType::TEXTURE_3D }
		});

	constexpr auto c_nri_texture_usage_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::TextureUsage, nri::TextureUsageBits>({
		{ Vadon::Render::RHI::TextureUsage::SHADER_RESOURCE, nri::TextureUsageBits::SHADER_RESOURCE },
		{ Vadon::Render::RHI::TextureUsage::READ_WRITE, nri::TextureUsageBits::SHADER_RESOURCE_STORAGE },
		{ Vadon::Render::RHI::TextureUsage::RENDER_TARGET, nri::TextureUsageBits::COLOR_ATTACHMENT },
		{ Vadon::Render::RHI::TextureUsage::DEPTH_STENCIL, nri::TextureUsageBits::DEPTH_STENCIL_ATTACHMENT }
		});

	constexpr auto c_nri_format_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::Format, nri::Format>({
		{ Vadon::Render::RHI::Format::UNKNOWN, nri::Format::UNKNOWN },
		{ Vadon::Render::RHI::Format::R8_UNORM, nri::Format::R8_UNORM },
		{ Vadon::Render::RHI::Format::R8_SNORM, nri::Format::R8_SNORM },
		{ Vadon::Render::RHI::Format::R8_UINT, nri::Format::R8_UINT },
		{ Vadon::Render::RHI::Format::R8_SINT, nri::Format::R8_SINT },
		{ Vadon::Render::RHI::Format::RG8_UNORM, nri::Format::RG8_UNORM },
		{ Vadon::Render::RHI::Format::RG8_SNORM, nri::Format::RG8_SNORM },
		{ Vadon::Render::RHI::Format::RG8_UINT, nri::Format::RG8_UINT },
		{ Vadon::Render::RHI::Format::RG8_SINT, nri::Format::RG8_SINT },
		{ Vadon::Render::RHI::Format::BGRA8_UNORM, nri::Format::BGRA8_UNORM },
		{ Vadon::Render::RHI::Format::BGRA8_SRGB, nri::Format::BGRA8_SRGB },
		{ Vadon::Render::RHI::Format::RGBA8_UNORM, nri::Format::RGBA8_UNORM },
		{ Vadon::Render::RHI::Format::RGBA8_SRGB, nri::Format::RGBA8_SRGB },
		{ Vadon::Render::RHI::Format::RGBA8_SNORM, nri::Format::RGBA8_SNORM },
		{ Vadon::Render::RHI::Format::RGBA8_UINT, nri::Format::RGBA8_UINT },
		{ Vadon::Render::RHI::Format::RGBA8_SINT, nri::Format::RGBA8_SINT },
		{ Vadon::Render::RHI::Format::R16_UNORM, nri::Format::R16_UNORM },
		{ Vadon::Render::RHI::Format::R16_SNORM, nri::Format::R16_SNORM },
		{ Vadon::Render::RHI::Format::R16_UINT, nri::Format::R16_UINT },
		{ Vadon::Render::RHI::Format::R16_SINT, nri::Format::R16_SINT },
		{ Vadon::Render::RHI::Format::R16_SFLOAT, nri::Format::R16_SFLOAT },
		{ Vadon::Render::RHI::Format::RG16_UNORM, nri::Format::RG16_UNORM },
		{ Vadon::Render::RHI::Format::RG16_SNORM, nri::Format::RG16_SNORM },
		{ Vadon::Render::RHI::Format::RG16_UINT, nri::Format::RG16_UINT },
		{ Vadon::Render::RHI::Format::RG16_SINT, nri::Format::RG16_SINT },
		{ Vadon::Render::RHI::Format::RG16_SFLOAT, nri::Format::RG16_SFLOAT },
		{ Vadon::Render::RHI::Format::RGBA16_UNORM, nri::Format::RGBA16_UNORM },
		{ Vadon::Render::RHI::Format::RGBA16_SNORM, nri::Format::RGBA16_SNORM },
		{ Vadon::Render::RHI::Format::RGBA16_UINT, nri::Format::RGBA16_UINT },
		{ Vadon::Render::RHI::Format::RGBA16_SINT, nri::Format::RGBA16_SINT },
		{ Vadon::Render::RHI::Format::RGBA16_SFLOAT, nri::Format::RGBA16_SFLOAT },
		{ Vadon::Render::RHI::Format::R32_UINT, nri::Format::R32_UINT },
		{ Vadon::Render::RHI::Format::R32_SINT, nri::Format::R32_SINT },
		{ Vadon::Render::RHI::Format::R32_SFLOAT, nri::Format::R32_SFLOAT },
		{ Vadon::Render::RHI::Format::RG32_UINT, nri::Format::RG32_UINT },
		{ Vadon::Render::RHI::Format::RG32_SINT, nri::Format::RG32_SINT },
		{ Vadon::Render::RHI::Format::RG32_SFLOAT, nri::Format::RG32_SFLOAT },
		{ Vadon::Render::RHI::Format::RGB32_UINT, nri::Format::RGB32_UINT },
		{ Vadon::Render::RHI::Format::RGB32_SINT, nri::Format::RGB32_SINT },
		{ Vadon::Render::RHI::Format::RGB32_SFLOAT, nri::Format::RGB32_SFLOAT },
		{ Vadon::Render::RHI::Format::RGBA32_UINT, nri::Format::RGBA32_UINT },
		{ Vadon::Render::RHI::Format::RGBA32_SINT, nri::Format::RGBA32_SINT },
		{ Vadon::Render::RHI::Format::RGBA32_SFLOAT, nri::Format::RGBA32_SFLOAT },
		{ Vadon::Render::RHI::Format::B5_G6_R5_UNORM, nri::Format::B5_G6_R5_UNORM },
		{ Vadon::Render::RHI::Format::B5_G5_R5_A1_UNORM, nri::Format::B5_G5_R5_A1_UNORM },
		{ Vadon::Render::RHI::Format::B4_G4_R4_A4_UNORM, nri::Format::B4_G4_R4_A4_UNORM },
		{ Vadon::Render::RHI::Format::R10_G10_B10_A2_UNORM, nri::Format::R10_G10_B10_A2_UNORM },
		{ Vadon::Render::RHI::Format::R10_G10_B10_A2_UINT, nri::Format::R10_G10_B10_A2_UINT },
		{ Vadon::Render::RHI::Format::R11_G11_B10_UFLOAT, nri::Format::R11_G11_B10_UFLOAT },
		{ Vadon::Render::RHI::Format::R9_G9_B9_E5_UFLOAT, nri::Format::R9_G9_B9_E5_UFLOAT },
		{ Vadon::Render::RHI::Format::NV12_UNORM, nri::Format::NV12_UNORM },
		{ Vadon::Render::RHI::Format::P010_UNORM, nri::Format::P010_UNORM },
		{ Vadon::Render::RHI::Format::P016_UNORM, nri::Format::P016_UNORM },
		{ Vadon::Render::RHI::Format::BC1_RGBA_UNORM, nri::Format::BC1_RGBA_UNORM },
		{ Vadon::Render::RHI::Format::BC1_RGBA_SRGB, nri::Format::BC1_RGBA_SRGB },
		{ Vadon::Render::RHI::Format::BC2_RGBA_UNORM, nri::Format::BC2_RGBA_UNORM },
		{ Vadon::Render::RHI::Format::BC2_RGBA_SRGB, nri::Format::BC2_RGBA_SRGB },
		{ Vadon::Render::RHI::Format::BC3_RGBA_UNORM, nri::Format::BC3_RGBA_UNORM },
		{ Vadon::Render::RHI::Format::BC3_RGBA_SRGB, nri::Format::BC3_RGBA_SRGB },
		{ Vadon::Render::RHI::Format::BC4_R_UNORM, nri::Format::BC4_R_UNORM },
		{ Vadon::Render::RHI::Format::BC4_R_SNORM, nri::Format::BC4_R_SNORM },
		{ Vadon::Render::RHI::Format::BC5_RG_UNORM, nri::Format::BC5_RG_UNORM },
		{ Vadon::Render::RHI::Format::BC5_RG_SNORM,nri::Format::BC5_RG_SNORM },
		{ Vadon::Render::RHI::Format::BC6H_RGB_UFLOAT, nri::Format::BC6H_RGB_UFLOAT },
		{ Vadon::Render::RHI::Format::BC6H_RGB_SFLOAT, nri::Format::BC6H_RGB_SFLOAT },
		{ Vadon::Render::RHI::Format::BC7_RGBA_UNORM, nri::Format::BC7_RGBA_UNORM },
		{ Vadon::Render::RHI::Format::BC7_RGBA_SRGB, nri::Format::BC7_RGBA_SRGB },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_UNORM, nri::Format::ETC2_RGB8_UNORM },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_SRGB, nri::Format::ETC2_RGB8_SRGB },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_A1_UNORM, nri::Format::ETC2_RGB8_A1_UNORM },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_A1_SRGB, nri::Format::ETC2_RGB8_A1_SRGB },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_A8_UNORM, nri::Format::ETC2_RGB8_A8_UNORM },
		{ Vadon::Render::RHI::Format::ETC2_RGB8_A8_SRGB, nri::Format::ETC2_RGB8_A8_SRGB },
		{ Vadon::Render::RHI::Format::ETC2_R11_UNORM, nri::Format::ETC2_R11_UNORM },
		{ Vadon::Render::RHI::Format::ETC2_R11_SNORM, nri::Format::ETC2_R11_SNORM },
		{ Vadon::Render::RHI::Format::ETC2_R11_G11_UNORM, nri::Format::ETC2_R11_G11_UNORM },
		{ Vadon::Render::RHI::Format::ETC2_R11_G11_SNORM, nri::Format::ETC2_R11_G11_SNORM },
		{ Vadon::Render::RHI::Format::ASTC_4X4_UNORM, nri::Format::ASTC_4X4_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_4X4_SRGB, nri::Format::ASTC_4X4_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_5X4_UNORM, nri::Format::ASTC_5X4_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_5X4_SRGB, nri::Format::ASTC_5X4_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_5X5_UNORM, nri::Format::ASTC_5X5_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_5X5_SRGB, nri::Format::ASTC_5X5_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_6X5_UNORM, nri::Format::ASTC_6X5_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_6X5_SRGB, nri::Format::ASTC_6X5_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_6X6_UNORM, nri::Format::ASTC_6X6_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_6X6_SRGB, nri::Format::ASTC_6X6_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_8X5_UNORM, nri::Format::ASTC_8X5_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_8X5_SRGB, nri::Format::ASTC_8X5_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_8X6_UNORM, nri::Format::ASTC_8X6_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_8X6_SRGB, nri::Format::ASTC_8X6_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_8X8_UNORM, nri::Format::ASTC_8X8_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_8X8_SRGB, nri::Format::ASTC_8X8_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_10X5_UNORM, nri::Format::ASTC_10X5_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_10X5_SRGB, nri::Format::ASTC_10X5_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_10X6_UNORM, nri::Format::ASTC_10X6_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_10X6_SRGB, nri::Format::ASTC_10X6_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_10X8_UNORM, nri::Format::ASTC_10X8_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_10X8_SRGB, nri::Format::ASTC_10X8_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_10X10_UNORM, nri::Format::ASTC_10X10_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_10X10_SRGB, nri::Format::ASTC_10X10_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_12X10_UNORM, nri::Format::ASTC_12X10_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_12X10_SRGB, nri::Format::ASTC_12X10_SRGB },
		{ Vadon::Render::RHI::Format::ASTC_12X12_UNORM, nri::Format::ASTC_12X12_UNORM },
		{ Vadon::Render::RHI::Format::ASTC_12X12_SRGB, nri::Format::ASTC_12X12_SRGB },
		{ Vadon::Render::RHI::Format::D16_UNORM, nri::Format::D16_UNORM },
		{ Vadon::Render::RHI::Format::D32_SFLOAT, nri::Format::D32_SFLOAT },
		{ Vadon::Render::RHI::Format::D24_UNORM_S8_UINT, nri::Format::D24_UNORM_S8_UINT },
		{ Vadon::Render::RHI::Format::D32_SFLOAT_S8_UINT, nri::Format::D32_SFLOAT_S8_UINT },
		});

	constexpr auto c_nri_shader_stage_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::ShaderStage, nri::StageBits>({
		{ Vadon::Render::RHI::ShaderStage::VERTEX_STAGE, nri::StageBits::VERTEX_SHADER },
		{ Vadon::Render::RHI::ShaderStage::PIXEL_STAGE, nri::StageBits::FRAGMENT_SHADER },
		{ Vadon::Render::RHI::ShaderStage::COMPUTE_STAGE, nri::StageBits::COMPUTE_SHADER }
	});

	constexpr auto c_nri_sampler_filter_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::SamplerFilter, nri::Filter>({
		{ Vadon::Render::RHI::SamplerFilter::NEAREST, nri::Filter::NEAREST },
		{ Vadon::Render::RHI::SamplerFilter::LINEAR, nri::Filter::LINEAR }
	});

	constexpr auto c_nri_sampler_filter_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::SamplerFilterOp, nri::FilterOp>({
		{ Vadon::Render::RHI::SamplerFilterOp::AVERAGE, nri::FilterOp::AVERAGE },
		{ Vadon::Render::RHI::SamplerFilterOp::MIN, nri::FilterOp::MIN },
		{ Vadon::Render::RHI::SamplerFilterOp::MAX, nri::FilterOp::MAX }
	});

	constexpr auto c_nri_sampler_address_mode_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::SamplerAddressMode, nri::AddressMode>({
		{ Vadon::Render::RHI::SamplerAddressMode::REPEAT, nri::AddressMode::REPEAT },
		{ Vadon::Render::RHI::SamplerAddressMode::MIRRORED_REPEAT, nri::AddressMode::MIRRORED_REPEAT },
		{ Vadon::Render::RHI::SamplerAddressMode::CLAMP_TO_EDGE, nri::AddressMode::CLAMP_TO_EDGE },
		{ Vadon::Render::RHI::SamplerAddressMode::CLAMP_TO_BORDER, nri::AddressMode::CLAMP_TO_BORDER },
		{ Vadon::Render::RHI::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE, nri::AddressMode::MIRROR_CLAMP_TO_EDGE }
	});

	constexpr auto c_nri_blend_factor_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::BlendFactor, nri::BlendFactor>({
		{ Vadon::Render::RHI::BlendFactor::ZERO, nri::BlendFactor::ZERO },
		{ Vadon::Render::RHI::BlendFactor::ONE, nri::BlendFactor::ONE },
		{ Vadon::Render::RHI::BlendFactor::SRC_COLOR, nri::BlendFactor::SRC_COLOR },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_SRC_COLOR, nri::BlendFactor::ONE_MINUS_SRC_COLOR },
		{ Vadon::Render::RHI::BlendFactor::DST_COLOR, nri::BlendFactor::DST_COLOR },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_DST_COLOR, nri::BlendFactor::ONE_MINUS_DST_COLOR },
		{ Vadon::Render::RHI::BlendFactor::SRC_ALPHA, nri::BlendFactor::SRC_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_SRC_ALPHA, nri::BlendFactor::ONE_MINUS_SRC_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::DST_ALPHA, nri::BlendFactor::DST_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_DST_ALPHA, nri::BlendFactor::ONE_MINUS_DST_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::CONSTANT_COLOR, nri::BlendFactor::CONSTANT_COLOR },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_CONSTANT_COLOR, nri::BlendFactor::ONE_MINUS_CONSTANT_COLOR },
		{ Vadon::Render::RHI::BlendFactor::CONSTANT_ALPHA, nri::BlendFactor::CONSTANT_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_CONSTANT_ALPHA, nri::BlendFactor::ONE_MINUS_CONSTANT_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::SRC_ALPHA_SATURATE, nri::BlendFactor::SRC_ALPHA_SATURATE },
		{ Vadon::Render::RHI::BlendFactor::SRC1_COLOR, nri::BlendFactor::SRC1_COLOR },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_SRC1_COLOR, nri::BlendFactor::ONE_MINUS_SRC1_COLOR },
		{ Vadon::Render::RHI::BlendFactor::SRC1_ALPHA, nri::BlendFactor::SRC1_ALPHA },
		{ Vadon::Render::RHI::BlendFactor::ONE_MINUS_SRC1_ALPHA, nri::BlendFactor::ONE_MINUS_SRC1_ALPHA }
	});

	constexpr auto c_nri_blend_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::BlendOperation, nri::BlendOp>({
		{ Vadon::Render::RHI::BlendOperation::ADD, nri::BlendOp::ADD },
		{ Vadon::Render::RHI::BlendOperation::SUBTRACT, nri::BlendOp::SUBTRACT },
		{ Vadon::Render::RHI::BlendOperation::REVERSE_SUBTRACT, nri::BlendOp::REVERSE_SUBTRACT },
		{ Vadon::Render::RHI::BlendOperation::MIN, nri::BlendOp::MIN },
		{ Vadon::Render::RHI::BlendOperation::MAX, nri::BlendOp::MAX }
	});

	constexpr auto c_nri_color_write_mapping = Vadon::Utilities::create_bit_flag_mapping<Vadon::Render::RHI::BlendColorWriteMask, nri::ColorWriteBits>({
		{ Vadon::Render::RHI::BlendColorWriteMask::COLOR_R, nri::ColorWriteBits::R },
		{ Vadon::Render::RHI::BlendColorWriteMask::COLOR_G, nri::ColorWriteBits::G },
		{ Vadon::Render::RHI::BlendColorWriteMask::COLOR_B, nri::ColorWriteBits::B },
		{ Vadon::Render::RHI::BlendColorWriteMask::COLOR_A, nri::ColorWriteBits::A }
	});

	constexpr auto c_nri_compare_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::CompareOperation, nri::CompareOp>({
		{ Vadon::Render::RHI::CompareOperation::NONE, nri::CompareOp::NONE },
		{ Vadon::Render::RHI::CompareOperation::ALWAYS, nri::CompareOp::ALWAYS },
		{ Vadon::Render::RHI::CompareOperation::NEVER, nri::CompareOp::NEVER },
		{ Vadon::Render::RHI::CompareOperation::EQUAL, nri::CompareOp::EQUAL },
		{ Vadon::Render::RHI::CompareOperation::NOT_EQUAL, nri::CompareOp::NOT_EQUAL },
		{ Vadon::Render::RHI::CompareOperation::LESS, nri::CompareOp::LESS },
		{ Vadon::Render::RHI::CompareOperation::LESS_EQUAL, nri::CompareOp::LESS_EQUAL },
		{ Vadon::Render::RHI::CompareOperation::GREATER, nri::CompareOp::GREATER },
		{ Vadon::Render::RHI::CompareOperation::GREATER_EQUAL, nri::CompareOp::GREATER_EQUAL },
	});

	constexpr auto c_nri_stencil_op_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::StencilOperation, nri::StencilOp>({
		{ Vadon::Render::RHI::StencilOperation::KEEP, nri::StencilOp::KEEP },
		{ Vadon::Render::RHI::StencilOperation::ZERO, nri::StencilOp::ZERO },
		{ Vadon::Render::RHI::StencilOperation::REPLACE, nri::StencilOp::REPLACE },
		{ Vadon::Render::RHI::StencilOperation::INCREMENT_AND_CLAMP, nri::StencilOp::INCREMENT_AND_CLAMP },
		{ Vadon::Render::RHI::StencilOperation::DECREMENT_AND_CLAMP, nri::StencilOp::DECREMENT_AND_CLAMP },
		{ Vadon::Render::RHI::StencilOperation::INVERT, nri::StencilOp::INVERT },
		{ Vadon::Render::RHI::StencilOperation::INCREMENT_AND_WRAP, nri::StencilOp::INCREMENT_AND_WRAP },
		{ Vadon::Render::RHI::StencilOperation::DECREMENT_AND_WRAP, nri::StencilOp::DECREMENT_AND_WRAP }
	});

	constexpr auto c_nri_fill_mode_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::RasterizerFillMode, nri::FillMode>({
		{ Vadon::Render::RHI::RasterizerFillMode::WIREFRAME, nri::FillMode::WIREFRAME },
		{ Vadon::Render::RHI::RasterizerFillMode::SOLID, nri::FillMode::SOLID }
	});

	constexpr auto c_nri_cull_mode_mapping = Vadon::Utilities::create_constexpr_map<Vadon::Render::RHI::RasterizerCullMode, nri::CullMode>({
		{ Vadon::Render::RHI::RasterizerCullMode::NONE, nri::CullMode::NONE },
		{ Vadon::Render::RHI::RasterizerCullMode::FRONT, nri::CullMode::FRONT },
		{ Vadon::Render::RHI::RasterizerCullMode::BACK, nri::CullMode::BACK }
	});

	struct NRIInterface
	{
		nri::CoreInterface core;
		nri::DescriptorHeapInterface descriptor_heap;
		nri::HelperInterface helper;
		nri::StreamerInterface streamer;
		nri::SwapChainInterface swap_chain;
	};

	nri::BufferDesc to_nri_buffer_desc(const Vadon::Render::RHI::BufferDescription& description)
	{
		nri::BufferDesc nri_desc = {};
		nri_desc.size = description.size;
		nri_desc.structureStride = description.structure_byte_stride;
		nri_desc.usage = c_nri_buffer_usage_mapping.convert(description.usage);
		nri_desc.byteAddress = Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::BYTE_ADDRESS);
		nri_desc.shared = false;

		return nri_desc;
	}

	nri::TextureDesc to_nri_texture_desc(const Vadon::Render::RHI::TextureDescription& description)
	{
		nri::TextureDesc nri_desc = {};
		nri_desc.type = c_nri_texture_type_mapping.at(description.type);
		nri_desc.usage = c_nri_texture_usage_mapping.convert(description.usage);
		nri_desc.format = c_nri_format_mapping.at(description.format);
		nri_desc.width = description.width;
		nri_desc.height = description.height;
		nri_desc.depth = description.depth;
		nri_desc.mipNum = description.mip_count;
		nri_desc.layerNum = description.layer_count;
		nri_desc.sampleNum = description.sample_count;

		return nri_desc;
	}

	nri::SamplerDesc to_nri_sampler_desc(const Vadon::Render::RHI::SamplerDescription& description)
	{
		nri::SamplerDesc nri_desc = {};
		nri_desc.filters.min = c_nri_sampler_filter_mapping.at(description.filters.min);
		nri_desc.filters.mag = c_nri_sampler_filter_mapping.at(description.filters.mag);
		nri_desc.filters.mip = c_nri_sampler_filter_mapping.at(description.filters.mip);
		nri_desc.filters.op = c_nri_sampler_filter_op_mapping.at(description.filters.op);

		nri_desc.addressModes.u = c_nri_sampler_address_mode_mapping.at(description.address_modes.u);
		nri_desc.addressModes.v = c_nri_sampler_address_mode_mapping.at(description.address_modes.v);
		nri_desc.addressModes.w = c_nri_sampler_address_mode_mapping.at(description.address_modes.w);

		nri_desc.anisotropy = description.anisotropy;

		nri_desc.mipBias = description.mip_bias;

		// TODO: other sampler settings?

		return nri_desc;
	}

	nri::BlendDesc to_nri_blend_desc(const Vadon::Render::RHI::ShaderBlendInfo& blend_info)
	{
		nri::BlendDesc nri_desc = {};
		nri_desc.srcFactor = c_nri_blend_factor_mapping.at(blend_info.source_factor);
		nri_desc.dstFactor = c_nri_blend_factor_mapping.at(blend_info.dest_factor);
		nri_desc.op = c_nri_blend_op_mapping.at(blend_info.operation);

		return nri_desc;
	}

	nri::DepthAttachmentDesc to_nri_depth_attachment_desc(const Vadon::Render::RHI::ShaderDepthInfo& depth_info)
	{
		nri::DepthAttachmentDesc nri_desc = {};
		nri_desc.compareOp = c_nri_compare_op_mapping.at(depth_info.compare_op);
		nri_desc.write = depth_info.write;
		nri_desc.boundsTest = depth_info.bounds_test;

		return nri_desc;
	}

	nri::StencilDesc to_nri_stencil_desc(const Vadon::Render::RHI::StencilInfo& stencil_info)
	{
		nri::StencilDesc nri_desc = {};
		nri_desc.compareOp = c_nri_compare_op_mapping.at(stencil_info.compare_op);
		nri_desc.failOp = c_nri_stencil_op_mapping.at(stencil_info.fail_op);
		nri_desc.passOp = c_nri_stencil_op_mapping.at(stencil_info.pass_op);
		nri_desc.depthFailOp = c_nri_stencil_op_mapping.at(stencil_info.depth_fail_op);
		nri_desc.writeMask = stencil_info.write_mask;
		nri_desc.compareMask = stencil_info.compare_mask;

		return nri_desc;
	}

	nri::StencilAttachmentDesc to_nri_stencil_attachment_desc(const Vadon::Render::RHI::ShaderStencilInfo& stencil_info)
	{
		nri::StencilAttachmentDesc nri_desc = {};
		nri_desc.front = to_nri_stencil_desc(stencil_info.front);
		nri_desc.back = to_nri_stencil_desc(stencil_info.back);

		return nri_desc;
	}

	nri::OutputMergerDesc to_nri_output_merger_desc(const Vadon::Render::RHI::ShaderOutputMergerInfo& output_merger_info, std::span<nri::ColorAttachmentDesc> nri_attachments)
	{
		nri::OutputMergerDesc nri_desc = {};
				
		for (size_t rt_index = 0; rt_index < output_merger_info.render_targets.size(); ++rt_index)
		{
			const Vadon::Render::RHI::ShaderRenderTargetInfo& current_rt_info = output_merger_info.render_targets[rt_index];
			nri::ColorAttachmentDesc& nri_rt_info = nri_attachments[rt_index];

			nri_rt_info.format = c_nri_format_mapping.at(current_rt_info.format);
			nri_rt_info.colorBlend = to_nri_blend_desc(current_rt_info.color_blend);
			nri_rt_info.alphaBlend = to_nri_blend_desc(current_rt_info.alpha_blend);
			nri_rt_info.colorWriteMask = c_nri_color_write_mapping.convert(current_rt_info.write_mask);
			nri_rt_info.blendEnabled = current_rt_info.blend_enabled;
		}

		nri_desc.colors = nri_attachments.data();
		nri_desc.colorNum = static_cast<uint32_t>(nri_attachments.size());

		nri_desc.depth = to_nri_depth_attachment_desc(output_merger_info.depth);
		nri_desc.stencil = to_nri_stencil_attachment_desc(output_merger_info.stencil);
		nri_desc.depthStencilFormat = c_nri_format_mapping.at(output_merger_info.depth_stencil_format);
		// TODO: additional features

		return nri_desc;
	}

	nri::RasterizationDesc to_nri_rasterization_desc(const Vadon::Render::RHI::ShaderRasterizerInfo& rasterizer_info)
	{
		nri::RasterizationDesc nri_desc = {};

		nri_desc.depthBias.constant = rasterizer_info.depth_bias.constant;
		nri_desc.depthBias.clamp = rasterizer_info.depth_bias.clamp;
		nri_desc.depthBias.slope = rasterizer_info.depth_bias.slope;

		nri_desc.fillMode = c_nri_fill_mode_mapping.at(rasterizer_info.fill_mode);
		nri_desc.cullMode = c_nri_cull_mode_mapping.at(rasterizer_info.cull_mode);
		nri_desc.frontCounterClockwise = rasterizer_info.front_counter_clockwise;
		nri_desc.depthClamp = rasterizer_info.depth_clamp;

		return nri_desc;
	}

	struct DescriptorSlotPool
	{
		std::vector<uint32_t> free_list;
		uint32_t slot_count = 0;

		uint32_t get_new_slot()
		{
			uint32_t slot_index = 0;
			if (free_list.empty() == false)
			{
				slot_index = free_list.back();
				free_list.pop_back();
			}
			else
			{
				slot_index = slot_count;
			}

			++slot_count;
			return slot_index;
		}

		void return_slot(uint32_t slot_index)
		{
			VADON_ASSERT(std::find(free_list.begin(), free_list.end(), slot_index) == free_list.end(), "Slot already in free list!");

			free_list.push_back(slot_index);
			--slot_count;
		}
	};

	struct NRIDescriptor
	{
		nri::Descriptor* nri_descriptor = nullptr;
		uint32_t heap_index = 0;

		bool is_valid() const { return nri_descriptor != nullptr; }

		void destroy(nri::CoreInterface& core, DescriptorSlotPool& descriptor_pool)
		{
			if (is_valid() == true)
			{
				descriptor_pool.return_slot(heap_index);
				heap_index = 0;

				core.DestroyDescriptor(nri_descriptor);
				nri_descriptor = nullptr;
			}
		}
	};

	struct NRIWriteResourceDescriptorHelper
	{
		std::span<nri::WriteResourceDescriptorsDesc> descriptors;
		uint32_t active_count = 0;

		void add_descriptor(NRIDescriptor& descriptor, DescriptorSlotPool& descriptor_pool)
		{
			if (descriptor.is_valid() == false)
			{
				return;
			}

			descriptor.heap_index = descriptor_pool.get_new_slot();

			nri::WriteResourceDescriptorsDesc& current_desc = descriptors[active_count];
			current_desc.resource = descriptor.nri_descriptor;
			current_desc.descriptorIndex = descriptor.heap_index;

			++active_count;
		}

		void write_descriptors(nri::DescriptorHeapInterface& nri_heap_interface, nri::DescriptorHeap& nri_heap)
		{
			if (active_count > 0)
			{
				NRI_ABORT_ON_FAILURE(nri_heap_interface.WriteResourceDescriptors(nri_heap, descriptors.data(), active_count));
			}
		}
	};

	struct NRIBuffer
	{
		Vadon::Render::RHI::BufferDescription description;
		nri::Buffer* nri_buffer;
		NRIDescriptor nri_srv;
		NRIDescriptor nri_uav;

		Vadon::Render::RHI::AccessStage last_access;

		// NOTE: if valid, this buffer belongs to a streamer
		Vadon::Render::RHI::StreamerHandle streamer;
	};

	struct NRICommandAllocator
	{
		Vadon::Render::RHI::QueueHandle queue;
		nri::CommandAllocator* nri_allocator;
	};

	struct NRICommandList
	{
		Vadon::Render::RHI::CommandAllocatorHandle allocator;
		nri::CommandBuffer* nri_command_buffer;
		Vadon::Private::Render::RHI::NRI::CommandList* nri_command_list = nullptr;
	};

	struct NRIFence
	{
		Vadon::Render::RHI::FenceDescription description;
		nri::Fence* nri_fence;
	};

	struct NRIGraphicsShader
	{
		Vadon::Render::RHI::GraphicsShaderDescription description;
		nri::Pipeline* nri_pipeline;
	};

	struct NRIQueue
	{
		Vadon::Render::RHI::QueueType type;
		uint32_t index;
	};

	struct NRISampler
	{
		Vadon::Render::RHI::SamplerDescription sampler_description = {};
		NRIDescriptor descriptor = {};
	};

	struct NRIStreamer
	{
		struct StaleEntry
		{
			Vadon::Render::RHI::BufferHandle buffer;
			uint32_t frame_count = 0;
		};

		Vadon::Render::RHI::StreamerDescription description = {};
		nri::Streamer* nri_streamer = nullptr;

		Vadon::Render::RHI::BufferHandle buffer;
		std::vector<StaleEntry> stale_entries;
	};

	struct NRISwapChainTexture
	{
		// NOTE: these fences are internal only, so we don't add them to the fence pool
		nri::Fence* acquire_semaphore;
		nri::Fence* release_semaphore;

		Vadon::Render::RHI::TextureHandle texture;
	};

	struct NRISwapChain
	{
		Vadon::Render::RHI::WindowDescription description = {};
		nri::SwapChain* nri_swapchain = nullptr;

		std::vector<NRISwapChainTexture> textures;
		uint32_t present_count = 0;
		int acquired_texture_index = -1;

		bool is_acquired() const { return (acquired_texture_index >= 0); }
	};

	struct NRITextureDescriptor
	{
		nri::TextureViewDesc nri_view_desc = {};
		NRIDescriptor descriptor = {};

		bool is_valid() const { return descriptor.is_valid(); }

		void destroy(nri::CoreInterface& core, DescriptorSlotPool& descriptor_pool)
		{
			descriptor.destroy(core, descriptor_pool);
		}
	};

	struct NRITexture
	{
		Vadon::Render::RHI::TextureDescription description;
		nri::Texture* nri_texture;
		NRITextureDescriptor nri_srv;
		NRITextureDescriptor nri_uav;
		NRITextureDescriptor nri_rtv;
		NRITextureDescriptor nri_dsv;
		
		// NOTE: if valid, this texture actually belongs to a swap chain
		Vadon::Render::RHI::WindowHandle window;

		Vadon::Render::RHI::AccessStage last_access;
	};

	struct NRIDescriptorHeap
	{
		nri::DescriptorHeap* nri_descriptor_heap = nullptr;

		DescriptorSlotPool resource_pool;
		DescriptorSlotPool sampler_pool;
	};
}

namespace Vadon::Private::Render::RHI::NRI
{
	struct Device::Internal
	{
		NRIInterface m_nri_interface;
		uint32_t m_adapter_index;

		nri::AllocationCallbacks m_allocation_callbacks;

		nri::Device* m_device;
		nri::Queue* m_graphics_queue;

		NRIDescriptorHeap m_descriptor_heap;

		nri::PipelineLayout* m_shared_layout;

		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Buffer, NRIBuffer> m_buffers;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::CommandAllocator, NRICommandAllocator> m_command_allocators;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::CommandList, NRICommandList> m_command_lists;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Fence, NRIFence> m_fences;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::GraphicsShader, NRIGraphicsShader> m_graphics_shaders;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Queue, NRIQueue> m_queues;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Sampler, NRISampler> m_samplers;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Streamer, NRIStreamer> m_streamers;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Texture, NRITexture> m_textures;
		Vadon::Utilities::ObjectPool<Vadon::Render::RHI::Window, NRISwapChain> m_windows;

		Internal()
			: m_adapter_index(0)
			, m_device(nullptr)
			, m_graphics_queue(nullptr)
			, m_shared_layout(nullptr)
		{
		}
		
		~Internal()
		{

		}

		bool initialize(Device& device)
		{
#if _DEBUG
			DebugAllocator::create_debug_allocator(m_allocation_callbacks);
#endif
			constexpr nri::VKBindingOffsets VK_BINDING_OFFSETS = { 0, 128, 32, 64 }; // Taken from NRIFramework

			nri::AdapterDesc adapter_descriptions[2] = {};
			uint32_t adapter_desc_count = 2;
			NRI_ABORT_ON_FAILURE(nri::nriEnumerateAdapters(adapter_descriptions, adapter_desc_count));

			const DeviceDescription& description = device.get_description();

			nri::DeviceCreationDesc nri_device_desc = {};
			nri_device_desc.graphicsAPI = c_nri_graphics_api_mapping.convert(description.graphics_api);
			nri_device_desc.enableGraphicsAPIValidation = description.enable_validation;
			nri_device_desc.enableNRIValidation = description.enable_backend_validation;
			nri_device_desc.enableD3D11CommandBufferEmulation = false;
			nri_device_desc.disableD3D12EnhancedBarriers = true;
			nri_device_desc.vkBindingOffsets = VK_BINDING_OFFSETS;
			nri_device_desc.adapterDesc = &adapter_descriptions[std::min(m_adapter_index, adapter_desc_count - 1)];
			nri_device_desc.allocationCallbacks = m_allocation_callbacks;

			if (!(nri_device_desc.adapterDesc->supportedGraphicsAPIs & nri_device_desc.graphicsAPI))
			{
				// TODO: send error that we do not support the wanted API!
				return false;
			}

			NRI_ABORT_ON_FAILURE(nri::nriCreateDevice(nri_device_desc, m_device));

			NRI_ABORT_ON_FAILURE(nri::nriGetInterface(*m_device, NRI_INTERFACE(nri::CoreInterface), &m_nri_interface.core));
			NRI_ABORT_ON_FAILURE(nri::nriGetInterface(*m_device, NRI_INTERFACE(nri::HelperInterface), &m_nri_interface.helper));
			NRI_ABORT_ON_FAILURE(nri::nriGetInterface(*m_device, NRI_INTERFACE(nri::StreamerInterface), &m_nri_interface.streamer));
			NRI_ABORT_ON_FAILURE(nri::nriGetInterface(*m_device, NRI_INTERFACE(nri::SwapChainInterface), &m_nri_interface.swap_chain));

			// Create descriptor heap based on device description
			{
				const nri::DescriptorHeapDesc descriptor_heap_desc = { description.max_resources, description.max_samplers };
				NRI_ABORT_ON_FAILURE(m_nri_interface.descriptor_heap.CreateDescriptorHeap(*m_device, descriptor_heap_desc, m_descriptor_heap.nri_descriptor_heap));
			}

			// Create shared root signature for all supported shaders
			{
				constexpr uint32_t c_root_constants_max_size = 16 * sizeof(uint32_t);
				const nri::RootConstantDesc root_constant = { 0, c_root_constants_max_size, nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER };

				nri::PipelineLayoutDesc pipeline_layout_desc = {};
				pipeline_layout_desc.rootRegisterSpace = 0;
				pipeline_layout_desc.rootConstants = &root_constant;
				pipeline_layout_desc.rootConstantNum = 1;
				pipeline_layout_desc.rootSamplers = nullptr;
				pipeline_layout_desc.rootSamplerNum = 0;
				pipeline_layout_desc.shaderStages = nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER;
				pipeline_layout_desc.flags = nri::PipelineLayoutBits::RESOURCE_HEAP_DIRECTLY_INDEXED | nri::PipelineLayoutBits::SAMPLER_HEAP_DIRECTLY_INDEXED;

				NRI_ABORT_ON_FAILURE(m_nri_interface.core.CreatePipelineLayout(*m_device, pipeline_layout_desc, m_shared_layout));
			}

			return true;
		}

		void shutdown()
		{
			m_nri_interface.core.DeviceWaitIdle(m_device);

			// TODO: check if we have leftover objects!
			m_nri_interface.core.DestroyPipelineLayout(m_shared_layout);

			nri::nriDestroyDevice(m_device);

#if _DEBUG
			if (m_allocation_callbacks.userArg != nullptr)
			{
				DebugAllocator::destroy_debug_allocator(m_allocation_callbacks);
			}
#endif
		}

		nri::Queue* get_nri_queue(QueueType type, uint32_t index) const
		{
			nri::Queue* nri_queue = nullptr;
			NRI_ABORT_ON_FAILURE(m_nri_interface.core.GetQueue(*m_device, c_nri_queue_type_mapping.at(type), index, nri_queue));

			return nri_queue;
		}

		BufferHandle get_streamer_buffer(StreamerHandle streamer_handle, NRIStreamer& streamer, nri::Buffer* nri_buffer)
		{
			if (streamer.buffer.is_valid() == true)
			{
				const NRIBuffer& streamer_buffer = m_buffers.get(streamer.buffer);
				if (streamer_buffer.nri_buffer == nri_buffer)
				{
					return streamer.buffer;
				}
			}

			// Need to add a new buffer
			BufferHandle new_buffer_handle = m_buffers.add();

			NRIBuffer& new_streamer_buffer = m_buffers.get(new_buffer_handle);
			new_streamer_buffer.nri_buffer = nri_buffer;
			new_streamer_buffer.streamer = streamer_handle;

			if (streamer.buffer.is_valid() == true)
			{
				NRIStreamer::StaleEntry stale_entry;
				stale_entry.buffer = streamer.buffer;
				stale_entry.frame_count = 0;

				streamer.stale_entries.push_back(stale_entry);
			}

			streamer.buffer = new_buffer_handle;

			return new_buffer_handle;
		}
	};

	Device::Device(const DeviceDescription& description)
		: Vadon::Render::RHI::Device(description)
		, m_internal(std::make_unique<Internal>())
	{
		// TODO: validate description?
	}

	Device::~Device() = default;

	BufferHandle Device::create_buffer(const BufferDescription& description)
	{
		const nri::BufferDesc nri_desc = to_nri_buffer_desc(description);

		nri::Buffer* nri_buffer = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateBuffer(*m_internal->m_device, nri_desc, nri_buffer));

		NRIDescriptor buffer_srv = {};
		NRIDescriptor buffer_uav = {};

		nri::BufferViewDesc nri_view_desc = {};
		nri_view_desc.buffer = nri_buffer;
		nri_view_desc.size = nri_desc.size;

		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::SHADER_RESOURCE) == true)
		{
			if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::STRUCTURED) == true)
			{
				nri_view_desc.type = nri::BufferView::STRUCTURED_BUFFER;
				nri_view_desc.structureStride = description.structure_byte_stride;
			}
			else if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::BYTE_ADDRESS) == true)
			{
				nri_view_desc.type = nri::BufferView::BYTE_ADDRESS_BUFFER;
			}
			else
			{
				nri_view_desc.type = nri::BufferView::BUFFER;
				nri_view_desc.format = c_nri_format_mapping.at(description.format);
			}
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateBufferView(nri_view_desc, buffer_srv.nri_descriptor));
		}
		else if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::CONSTANT) == true)
		{
			nri_view_desc.type = nri::BufferView::CONSTANT_BUFFER;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateBufferView(nri_view_desc, buffer_srv.nri_descriptor));
		}

		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::READ_WRITE) == true)
		{
			if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::STRUCTURED) == true)
			{
				nri_view_desc.type = nri::BufferView::STORAGE_STRUCTURED_BUFFER;
				nri_view_desc.structureStride = description.structure_byte_stride;
			}
			else if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::BufferUsage::BYTE_ADDRESS) == true)
			{
				nri_view_desc.type = nri::BufferView::STORAGE_BYTE_ADDRESS_BUFFER;
			}
			else
			{
				nri_view_desc.type = nri::BufferView::STORAGE_BUFFER;
				nri_view_desc.format = c_nri_format_mapping.at(description.format);
			}
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateBufferView(nri_view_desc, buffer_uav.nri_descriptor));
		}

		nri::WriteResourceDescriptorsDesc resources[2] = {};
		NRIWriteResourceDescriptorHelper descriptor_helper;
		descriptor_helper.descriptors = resources;

		descriptor_helper.add_descriptor(buffer_srv, m_internal->m_descriptor_heap.resource_pool);
		descriptor_helper.add_descriptor(buffer_uav, m_internal->m_descriptor_heap.resource_pool);

		descriptor_helper.write_descriptors(m_internal->m_nri_interface.descriptor_heap, *m_internal->m_descriptor_heap.nri_descriptor_heap);

		BufferHandle new_buffer_handle = m_internal->m_buffers.add();
		NRIBuffer& new_buffer = m_internal->m_buffers.get(new_buffer_handle);

		new_buffer.description = description;
		new_buffer.nri_buffer = nri_buffer;
		new_buffer.nri_srv = buffer_srv;
		new_buffer.nri_uav = buffer_uav;

		return new_buffer_handle;
	}

	void Device::remove_buffer(BufferHandle buffer_handle)
	{
		NRIBuffer& buffer = m_internal->m_buffers.get(buffer_handle);

		buffer.nri_srv.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);
		buffer.nri_uav.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);

		m_internal->m_nri_interface.core.DestroyBuffer(buffer.nri_buffer);

		buffer.nri_buffer = nullptr;

		m_internal->m_buffers.remove(buffer_handle);
	}

	CommandAllocatorHandle Device::create_command_allocator(QueueHandle queue_handle)
	{
		const NRIQueue& queue = m_internal->m_queues.get(queue_handle);
		
		nri::Queue* nri_queue = m_internal->get_nri_queue(queue.type, queue.index);

		nri::CommandAllocator* nri_allocator = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateCommandAllocator(*nri_queue, nri_allocator));

		CommandAllocatorHandle new_allocator_handle = m_internal->m_command_allocators.add();
		NRICommandAllocator& new_allocator = m_internal->m_command_allocators.get(new_allocator_handle);

		new_allocator.nri_allocator = nri_allocator;
		new_allocator.queue = queue_handle;

		return new_allocator_handle;
	}

	void Device::reset_command_allocator(CommandAllocatorHandle allocator_handle)
	{
		const NRICommandAllocator& allocator = m_internal->m_command_allocators.get(allocator_handle);
		m_internal->m_nri_interface.core.ResetCommandAllocator(*allocator.nri_allocator);
	}

	void Device::remove_command_allocator(CommandAllocatorHandle allocator_handle)
	{
		NRICommandAllocator& allocator = m_internal->m_command_allocators.get(allocator_handle);

		m_internal->m_nri_interface.core.DestroyCommandAllocator(allocator.nri_allocator);

		allocator.nri_allocator = nullptr;

		m_internal->m_command_allocators.remove(allocator_handle);
	}

	CommandListHandle Device::create_command_list(CommandAllocatorHandle allocator_handle)
	{
		NRICommandAllocator& allocator = m_internal->m_command_allocators.get(allocator_handle);

		nri::CommandBuffer* nri_command_buffer = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateCommandBuffer(*allocator.nri_allocator, nri_command_buffer));

		CommandListHandle new_command_list_handle = m_internal->m_command_lists.add();
		NRICommandList& new_command_list = m_internal->m_command_lists.get(new_command_list_handle);

		new_command_list.allocator = allocator_handle;
		new_command_list.nri_command_buffer = nri_command_buffer;

		return new_command_list_handle;
	}

	Vadon::Render::RHI::CommandList* Device::get_command_list(CommandListHandle command_list_handle)
	{
		NRICommandList& command_list = m_internal->m_command_lists.get(command_list_handle);
		VADON_ASSERT(command_list.nri_command_list == nullptr, "Command list already in use!");

		command_list.nri_command_list = new CommandList(*this, command_list_handle, command_list.nri_command_buffer);

		return command_list.nri_command_list;
	}

	void Device::return_command_list(Vadon::Render::RHI::CommandList* command_list)
	{
		CommandList* nri_command_list = static_cast<CommandList*>(command_list);

		NRICommandList& command_list_data = m_internal->m_command_lists.get(nri_command_list->get_command_list_handle());
		VADON_ASSERT(command_list_data.nri_command_list == nri_command_list, "Command list data mismatch!");

		delete nri_command_list;
		command_list_data.nri_command_list = nullptr;
	}

	void Device::remove_command_list(CommandListHandle command_list_handle)
	{
		NRICommandList& command_list = m_internal->m_command_lists.get(command_list_handle);

		m_internal->m_nri_interface.core.DestroyCommandBuffer(command_list.nri_command_buffer);

		command_list.nri_command_buffer = nullptr;

		m_internal->m_command_lists.remove(command_list_handle);
	}

	FenceHandle Device::create_fence(const FenceDescription& description)
	{
		nri::FenceDesc nri_desc;
		nri_desc.initialValue = description.initial_value;
		nri_desc.shared = description.shared;

		nri::Fence* nri_fence = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateFence(*m_internal->m_device, nri_desc, nri_fence));

		FenceHandle new_fence_handle = m_internal->m_fences.add();
		NRIFence& new_fence = m_internal->m_fences.get(new_fence_handle);

		new_fence.description = description;
		new_fence.nri_fence = nri_fence;

		return new_fence_handle;
	}

	void Device::wait_fence(FenceHandle fence_handle, uint64_t value)
	{
		const NRIFence& fence = m_internal->m_fences.get(fence_handle);
		m_internal->m_nri_interface.core.Wait(*fence.nri_fence, value);
	}

	uint64_t Device::get_fence_value(FenceHandle fence_handle)
	{
		const NRIFence& fence = m_internal->m_fences.get(fence_handle);
		return m_internal->m_nri_interface.core.GetFenceValue(*fence.nri_fence);
	}

	void Device::remove_fence(FenceHandle fence_handle)
	{
		NRIFence& fence = m_internal->m_fences.get(fence_handle);

		m_internal->m_nri_interface.core.DestroyFence(fence.nri_fence);

		fence.nri_fence = nullptr;

		m_internal->m_fences.remove(fence_handle);
	}

	SamplerHandle Device::create_sampler(const SamplerDescription& description)
	{
		// FIXME: use sampler lookup to ensure we don't duplicate identical samplers?
		const nri::SamplerDesc nri_desc = to_nri_sampler_desc(description);

		nri::Descriptor* nri_sampler = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateSampler(*m_internal->m_device, nri_desc, nri_sampler));

		SamplerHandle new_sampler_handle = m_internal->m_samplers.add();
		NRISampler& new_sampler = m_internal->m_samplers.get(new_sampler_handle);

		new_sampler.descriptor.nri_descriptor = nri_sampler;

		return new_sampler_handle;
	}

	void Device::remove_sampler(SamplerHandle sampler_handle)
	{
		NRISampler& sampler = m_internal->m_samplers.get(sampler_handle);

		sampler.descriptor.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.sampler_pool);

		m_internal->m_samplers.remove(sampler_handle);
	}

	StreamerHandle Device::create_streamer(const StreamerDescription& description)
	{
		nri::StreamerDesc nri_desc = {};
		nri_desc.constantBufferMemoryLocation = nri::MemoryLocation::HOST_UPLOAD;
		nri_desc.constantBufferSize = description.constant_buffer_size;
		nri_desc.dynamicBufferMemoryLocation = nri::MemoryLocation::HOST_UPLOAD;
		nri_desc.dynamicBufferDesc = to_nri_buffer_desc(description.dynamic_buffer_description);
		nri_desc.queuedFrameNum = description.queued_frames;

		nri::Streamer* nri_streamer = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.streamer.CreateStreamer(*m_internal->m_device, nri_desc, nri_streamer));

		StreamerHandle new_streamer_handle = m_internal->m_streamers.add();
		NRIStreamer& new_streamer = m_internal->m_streamers.get(new_streamer_handle);
		new_streamer.description = description;
		new_streamer.nri_streamer = nri_streamer;

		return new_streamer_handle;
	}

	uint32_t Device::stream_constant_data(StreamerHandle streamer_handle, const void* data, uint32_t data_size)
	{
		const NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);
		return m_internal->m_nri_interface.streamer.StreamConstantData(*streamer.nri_streamer, data, data_size);
	}

	StreamerCopyBatch Device::begin_streamer_copy_batch(StreamerHandle streamer_handle)
	{
		const NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);
		return m_internal->m_nri_interface.streamer.BeginStreamerCopyBatch(*streamer.nri_streamer);
	}

	BufferOffset Device::stream_buffer_data(StreamerHandle streamer_handle, const StreamBufferDataDescription& description)
	{
		std::array<nri::DataSize, 64> nri_data_sizes;
		VADON_ASSERT(description.batches.size() <= nri_data_sizes.size(), "Too many batches!");

		for (size_t batch_index = 0; batch_index < nri_data_sizes.size(); ++batch_index)
		{
			const Vadon::Render::RHI::DataBatch& current_batch = description.batches[batch_index];

			nri::DataSize& nri_data_size = nri_data_sizes[batch_index];
			nri_data_size.data = current_batch.data;
			nri_data_size.size = current_batch.size;
		}

		nri::StreamBufferDataDesc nri_desc = {};
		nri_desc.dataChunks = nri_data_sizes.data();
		nri_desc.dataChunkNum = static_cast<uint32_t>(description.batches.size());
		nri_desc.placementAlignment = description.alignment;
		nri_desc.copyBatch = description.copy_batch;
		nri_desc.dstBuffer = m_internal->m_buffers.get(description.destination_buffer).nri_buffer;
		nri_desc.dstOffset = description.destination_offset;

		NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);
		const nri::BufferOffset nri_offset = m_internal->m_nri_interface.streamer.StreamBufferData(*streamer.nri_streamer, nri_desc);

		BufferOffset buffer_offset = {};
		buffer_offset.buffer = m_internal->get_streamer_buffer(streamer_handle, streamer, nri_offset.buffer);
		buffer_offset.offset = nri_offset.offset;

		return buffer_offset;
	}

	BufferOffset Device::stream_texture_data(StreamerHandle streamer_handle, const StreamTextureDataDescription& description)
	{
		nri::StreamTextureDataDesc nri_desc = {};
		nri_desc.data = description.data;
		nri_desc.dataRowPitch = description.data_row_pitch;
		nri_desc.dataSlicePitch = description.data_slice_pitch;
		nri_desc.copyBatch = description.copy_batch;
		nri_desc.dstTexture = m_internal->m_textures.get(description.destination_texture).nri_texture;

		NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);

		const nri::BufferOffset nri_offset = m_internal->m_nri_interface.streamer.StreamTextureData(*streamer.nri_streamer, nri_desc);

		BufferOffset buffer_offset = {};
		buffer_offset.buffer = m_internal->get_streamer_buffer(streamer_handle, streamer, nri_offset.buffer);
		buffer_offset.offset = nri_offset.offset;

		return buffer_offset;
	}

	void Device::end_streamer_frame(StreamerHandle streamer_handle)
	{
		NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);

		for (size_t entry_index = 0; entry_index < streamer.stale_entries.size();)
		{
			NRIStreamer::StaleEntry& current_entry = streamer.stale_entries[entry_index];
			++current_entry.frame_count;

			if (current_entry.frame_count >= streamer.description.queued_frames)
			{
				NRIBuffer& nri_buffer = m_internal->m_buffers.get(current_entry.buffer);
				nri_buffer.nri_buffer = nullptr;
				nri_buffer.streamer.invalidate();

				m_internal->m_buffers.remove(current_entry.buffer);

				streamer.stale_entries[entry_index] = std::move(streamer.stale_entries.back());
				streamer.stale_entries.pop_back();
			}
			else
			{
				++entry_index;
			}
		}
	}

	void Device::remove_streamer(StreamerHandle streamer_handle)
	{
		NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);
		m_internal->m_nri_interface.streamer.DestroyStreamer(streamer.nri_streamer);

		streamer.nri_streamer = nullptr;

		m_internal->m_streamers.remove(streamer_handle);
	}

	nri::Streamer& Device::get_nri_streamer(StreamerHandle streamer_handle)
	{
		NRIStreamer& streamer = m_internal->m_streamers.get(streamer_handle);
		return *streamer.nri_streamer;
	}

	nri::Buffer* Device::get_nri_buffer(BufferHandle buffer_handle)
	{
		return m_internal->m_buffers.get(buffer_handle).nri_buffer;
	}

	nri::Texture* Device::get_nri_texture(TextureHandle texture_handle)
	{
		return m_internal->m_textures.get(texture_handle).nri_texture;
	}

	nri::Descriptor* Device::get_nri_texture_rtv(TextureHandle texture_handle)
	{
		return m_internal->m_textures.get(texture_handle).nri_rtv.descriptor.nri_descriptor;
	}

	nri::StageBits Device::get_nri_stage_bits(Vadon::Render::RHI::ShaderStage stages)
	{
		return c_nri_shader_stage_mapping.convert(stages);
	}

	GraphicsShaderHandle Device::create_graphics_shader(const GraphicsShaderDescription& description)
	{
		nri::GraphicsPipelineDesc nri_desc = {};

		std::array<nri::ColorAttachmentDesc, 8> nri_attachments;

		nri_desc.outputMerger = to_nri_output_merger_desc(description.output_merger, std::span(nri_attachments));

		std::array<nri::ShaderDesc, 24> nri_shader_descs;
		for (size_t stage_index = 0; stage_index < description.stages.size(); ++stage_index)
		{
			const Vadon::Render::RHI::ShaderStageDescription& current_stage = description.stages[stage_index];
			nri::ShaderDesc& nri_stage_desc = nri_shader_descs[stage_index];

			nri_stage_desc.stage = c_nri_shader_stage_mapping.convert(current_stage.stage);
			nri_stage_desc.bytecode = current_stage.bytecode.data();
			nri_stage_desc.size = current_stage.bytecode.size();
		}

		nri_desc.shaderNum = static_cast<uint32_t>(description.stages.size());

		nri::Pipeline* nri_pipeline = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateGraphicsPipeline(*m_internal->m_device, nri_desc, nri_pipeline));

		GraphicsShaderHandle new_shader_handle = m_internal->m_graphics_shaders.add();
		NRIGraphicsShader& new_shader = m_internal->m_graphics_shaders.get(new_shader_handle);

		new_shader.description = description;
		new_shader.nri_pipeline = nri_pipeline;

		return new_shader_handle;
	}

	void Device::remove_graphics_shader(GraphicsShaderHandle shader_handle)
	{
		NRIGraphicsShader& shader = m_internal->m_graphics_shaders.get(shader_handle);

		m_internal->m_nri_interface.core.DestroyPipeline(shader.nri_pipeline);

		shader.nri_pipeline = nullptr;

		m_internal->m_graphics_shaders.remove(shader_handle);
	}

	TextureHandle Device::create_texture(const TextureDescription& description)
	{
		const nri::TextureDesc nri_desc = to_nri_texture_desc(description);

		nri::Texture* nri_texture = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTexture(*m_internal->m_device, nri_desc, nri_texture));

		nri::TextureViewDesc nri_view_desc = {};
		nri_view_desc.texture = nri_texture;
		nri_view_desc.format = c_nri_format_mapping.at(description.format);

		NRITextureDescriptor nri_srv_descriptor = {};
		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::TextureUsage::SHADER_RESOURCE) == true)
		{
			nri_view_desc.type = nri::TextureView::TEXTURE;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTextureView(nri_view_desc, nri_srv_descriptor.descriptor.nri_descriptor));

			nri_srv_descriptor.nri_view_desc = nri_view_desc;
		}

		NRITextureDescriptor nri_uav_descriptor = {};
		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::TextureUsage::READ_WRITE) == true)
		{
			nri_view_desc.type = nri::TextureView::STORAGE_TEXTURE;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTextureView(nri_view_desc, nri_uav_descriptor.descriptor.nri_descriptor));

			nri_uav_descriptor.nri_view_desc = nri_view_desc;
		}

		NRITextureDescriptor nri_rtv_descriptor = {};
		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::TextureUsage::RENDER_TARGET) == true)
		{
			nri_view_desc.type = nri::TextureView::COLOR_ATTACHMENT;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTextureView(nri_view_desc, nri_rtv_descriptor.descriptor.nri_descriptor));

			nri_rtv_descriptor.nri_view_desc = nri_view_desc;
		}

		NRITextureDescriptor nri_dsv_descriptor = {};
		if (Vadon::Utilities::enum_has_any_flag(description.usage, Vadon::Render::RHI::TextureUsage::RENDER_TARGET) == true)
		{
			nri_view_desc.type = nri::TextureView::DEPTH_STENCIL_ATTACHMENT;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTextureView(nri_view_desc, nri_dsv_descriptor.descriptor.nri_descriptor));

			nri_dsv_descriptor.nri_view_desc = nri_view_desc;
		}

		nri::WriteResourceDescriptorsDesc resources[4] = {};

		NRIWriteResourceDescriptorHelper descriptor_helper;
		descriptor_helper.descriptors = resources;

		descriptor_helper.add_descriptor(nri_srv_descriptor.descriptor, m_internal->m_descriptor_heap.resource_pool);
		descriptor_helper.add_descriptor(nri_uav_descriptor.descriptor, m_internal->m_descriptor_heap.resource_pool);
		descriptor_helper.add_descriptor(nri_rtv_descriptor.descriptor, m_internal->m_descriptor_heap.resource_pool);
		descriptor_helper.add_descriptor(nri_dsv_descriptor.descriptor, m_internal->m_descriptor_heap.resource_pool);

		descriptor_helper.write_descriptors(m_internal->m_nri_interface.descriptor_heap, *m_internal->m_descriptor_heap.nri_descriptor_heap);

		TextureHandle new_texture_handle = m_internal->m_textures.add();
		NRITexture& new_texture = m_internal->m_textures.get(new_texture_handle);

		new_texture.description = description;
		new_texture.nri_texture = nri_texture;
		new_texture.nri_srv = nri_srv_descriptor;
		new_texture.nri_uav = nri_uav_descriptor;
		new_texture.nri_rtv = nri_rtv_descriptor;
		new_texture.nri_dsv = nri_dsv_descriptor;

		return new_texture_handle;
	}

	void Device::remove_texture(TextureHandle texture_handle)
	{
		NRITexture& texture = m_internal->m_textures.get(texture_handle);

		VADON_ASSERT(texture.window.is_valid() == false, "Cannot directly delete swap chain texture!");

		texture.nri_dsv.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);
		texture.nri_rtv.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);
		texture.nri_uav.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);
		texture.nri_srv.destroy(m_internal->m_nri_interface.core, m_internal->m_descriptor_heap.resource_pool);

		m_internal->m_nri_interface.core.DestroyTexture(texture.nri_texture);

		texture.nri_texture = nullptr;

		m_internal->m_textures.remove(texture_handle);
	}

	QueueHandle Device::get_queue(QueueType type, uint32_t index)
	{
		for (const auto queue_pair : m_internal->m_queues)
		{
			const NRIQueue* current_queue = queue_pair.second;
			if (current_queue->type == type && current_queue->index == index)
			{
				return queue_pair.first;
			}
		}

		nri::Queue* nri_queue = m_internal->get_nri_queue(type, index);
		if (nri_queue == nullptr)
		{
			VADON_ERROR("Failed to get queue!");
			return QueueHandle();
		}

		QueueHandle new_queue_handle = m_internal->m_queues.add();
		NRIQueue& new_queue = m_internal->m_queues.get(new_queue_handle);

		new_queue.type = type;
		new_queue.index = index;

		return new_queue_handle;
	}

	void Device::queue_submit(QueueHandle queue_handle, const QueueSubmitDescription& queue_submit)
	{
		std::array<nri::FenceSubmitDesc, 32> nri_wait_fences;
		std::array<nri::CommandBuffer*, 32> nri_command_buffers;
		std::array<nri::FenceSubmitDesc, 32> nri_signal_fences;

		VADON_ASSERT(queue_submit.wait_fences.size() <= nri_wait_fences.size(), "Too many wait fences!");
		VADON_ASSERT(queue_submit.command_lists.size() <= nri_command_buffers.size(), "Too many command lists!");
		VADON_ASSERT(queue_submit.signal_fences.size() <= nri_signal_fences.size(), "Too many signal fences!");

		for (size_t fence_index = 0; fence_index < queue_submit.wait_fences.size(); ++fence_index)
		{
			const Vadon::Render::RHI::FenceSubmitDescription& submit_desc = queue_submit.wait_fences[fence_index];
			const NRIFence& fence = m_internal->m_fences.get(submit_desc.fence);

			nri::FenceSubmitDesc& wait_fence_desc = nri_wait_fences[fence_index];
			wait_fence_desc.fence = fence.nri_fence;
			wait_fence_desc.value = submit_desc.value;
			// TODO: stage bits
		}

		for (size_t command_list_index = 0; command_list_index < queue_submit.command_lists.size(); ++command_list_index)
		{
			const NRICommandList& command_list = m_internal->m_command_lists.get(queue_submit.command_lists[command_list_index]);
			nri_command_buffers[command_list_index] = command_list.nri_command_buffer;
		}

		for (size_t fence_index = 0; fence_index < queue_submit.signal_fences.size(); ++fence_index)
		{
			const Vadon::Render::RHI::FenceSubmitDescription& submit_desc = queue_submit.signal_fences[fence_index];
			const NRIFence& fence = m_internal->m_fences.get(submit_desc.fence);

			nri::FenceSubmitDesc& signal_fence_desc = nri_signal_fences[fence_index];
			signal_fence_desc.fence = fence.nri_fence;
			signal_fence_desc.value = submit_desc.value;
			// TODO: stage bits
		}

		nri::QueueSubmitDesc nri_desc = {};

		nri_desc.waitFences = nri_wait_fences.data();
		nri_desc.waitFenceNum = static_cast<uint32_t>(queue_submit.wait_fences.size());
		nri_desc.commandBuffers = nri_command_buffers.data();
		nri_desc.commandBufferNum = static_cast<uint32_t>(queue_submit.command_lists.size());
		nri_desc.signalFences = nri_signal_fences.data();
		nri_desc.signalFenceNum = static_cast<uint32_t>(queue_submit.signal_fences.size());

		// TODO: swap chain?

		const NRIQueue& queue = m_internal->m_queues.get(queue_handle);
		nri::Queue* nri_queue = m_internal->get_nri_queue(queue.type, queue.index);

		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.QueueSubmit(*nri_queue, nri_desc));
	}

	void Device::queue_wait_idle(QueueHandle queue_handle)
	{
		const NRIQueue& queue = m_internal->m_queues.get(queue_handle);
		nri::Queue* nri_queue = m_internal->get_nri_queue(queue.type, queue.index);

		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.QueueWaitIdle(nri_queue));
	}

	WindowHandle Device::create_window(const WindowDescription& description)
	{
		const NRIQueue& queue = m_internal->m_queues.get(description.queue);
		nri::Queue* nri_queue = m_internal->get_nri_queue(queue.type, queue.index);

		nri::SwapChainDesc nri_desc = {};
		nri_desc.window.windows.hwnd = description.platform_handle.window_handle;
		nri_desc.width = description.width;
		nri_desc.height = description.height;
		nri_desc.queue = nri_queue;
		nri_desc.format = nri::SwapChainFormat::BT709_G22_8BIT;
		nri_desc.flags = nri::SwapChainBits::NONE;
		if (description.vsync)
		{
			nri_desc.flags |= nri::SwapChainBits::VSYNC;
		}
		if (description.allow_tearing)
		{
			nri_desc.flags |= nri::SwapChainBits::ALLOW_TEARING;
		}
		nri_desc.queuedFrameNum = description.vsync ? 2 : 3;
		nri_desc.textureNum = nri_desc.queuedFrameNum + 1;

		nri::SwapChain* nri_swap_chain = nullptr;
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.swap_chain.CreateSwapChain(*m_internal->m_device, nri_desc, nri_swap_chain));

		uint32_t swap_chain_texture_num = 0;
		nri::Texture* const* swap_chain_textures = m_internal->m_nri_interface.swap_chain.GetSwapChainTextures(*nri_swap_chain, swap_chain_texture_num);

		const nri::Format swap_chain_format = m_internal->m_nri_interface.core.GetTextureDesc(*swap_chain_textures[0]).format;

		std::vector<NRITexture> temp_textures;
		std::vector<NRISwapChainTexture> temp_swap_chain_textures;

		for (uint32_t texture_index = 0; texture_index < swap_chain_texture_num; ++texture_index)
		{
			nri::TextureViewDesc texture_view_desc = { swap_chain_textures[texture_index], nri::TextureView::COLOR_ATTACHMENT, swap_chain_format };

			nri::Descriptor* rtv_descriptor = nullptr;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateTextureView(texture_view_desc, rtv_descriptor));

			nri::Fence* acquire_semaphore = nullptr;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateFence(*m_internal->m_device, { nri::SWAPCHAIN_SEMAPHORE }, acquire_semaphore));

			nri::Fence* release_semaphore = nullptr;
			NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.core.CreateFence(*m_internal->m_device, { nri::SWAPCHAIN_SEMAPHORE }, release_semaphore));

			NRITexture temp_texture;
			temp_texture.nri_texture = swap_chain_textures[texture_index];
			temp_texture.nri_rtv.nri_view_desc = texture_view_desc;
			temp_texture.nri_rtv.descriptor.nri_descriptor = rtv_descriptor;

			temp_textures.push_back(temp_texture);

			NRISwapChainTexture temp_sc_texture = {};
			temp_sc_texture.acquire_semaphore = acquire_semaphore;
			temp_sc_texture.release_semaphore = release_semaphore;

			temp_swap_chain_textures.push_back(temp_sc_texture);
		}

		// Everything succeeded, now we add objects to the pool
		WindowHandle new_window_handle = m_internal->m_windows.add();

		for (uint32_t texture_index = 0; texture_index < swap_chain_texture_num; ++texture_index)
		{
			const NRITexture& temp_texture = temp_textures[texture_index];
			NRISwapChainTexture& temp_sc_texture = temp_swap_chain_textures[texture_index];

			TextureHandle sc_texture_handle = m_internal->m_textures.add();
			NRITexture& current_texture = m_internal->m_textures.get(sc_texture_handle);
			current_texture = temp_texture;
			current_texture.window = new_window_handle;

			temp_sc_texture.texture = sc_texture_handle;
		}

		NRISwapChain& new_swap_chain = m_internal->m_windows.get(new_window_handle);

		new_swap_chain.description = description;
		new_swap_chain.nri_swapchain = nri_swap_chain;
		new_swap_chain.textures = temp_swap_chain_textures;

		return new_window_handle;
	}

	TextureHandle Device::get_window_next_texture(WindowHandle window_handle)
	{
		NRISwapChain& swap_chain = m_internal->m_windows.get(window_handle);

		VADON_ASSERT(swap_chain.is_acquired() == false, "Swap chain texture already acquired for this frame!");

		uint32_t semaphore_index = swap_chain.present_count % static_cast<uint32_t>(swap_chain.textures.size());
		nri::Fence* acquire_semaphore = swap_chain.textures[semaphore_index].acquire_semaphore;

		uint32_t current_swap_chain_texture_index = 0;
		m_internal->m_nri_interface.swap_chain.AcquireNextTexture(*swap_chain.nri_swapchain, *acquire_semaphore, current_swap_chain_texture_index);

		swap_chain.acquired_texture_index = static_cast<int>(current_swap_chain_texture_index);

		const NRISwapChainTexture& swap_chain_texture = swap_chain.textures[current_swap_chain_texture_index];
		return swap_chain_texture.texture;
	}

	void Device::present_window(WindowHandle window_handle)
	{
		NRISwapChain& swap_chain = m_internal->m_windows.get(window_handle);

		VADON_ASSERT(swap_chain.is_acquired() == true, "Did not acquire swap chain for this frame!");

		const NRISwapChainTexture& swap_chain_texture = swap_chain.textures[swap_chain.acquired_texture_index];
		NRI_ABORT_ON_FAILURE(m_internal->m_nri_interface.swap_chain.QueuePresent(*swap_chain.nri_swapchain, *swap_chain_texture.release_semaphore, 0));

		++swap_chain.present_count;
	}

	void Device::remove_window(WindowHandle window_handle)
	{
		NRISwapChain& swap_chain = m_internal->m_windows.get(window_handle);

		for (NRISwapChainTexture& current_texture : swap_chain.textures)
		{
			// Destroy semaphores
			m_internal->m_nri_interface.core.DestroyFence(current_texture.acquire_semaphore);
			m_internal->m_nri_interface.core.DestroyFence(current_texture.release_semaphore);

			NRITexture& current_texture_data = m_internal->m_textures.get(current_texture.texture);
			VADON_ASSERT(current_texture_data.window.is_valid() == false, "Invalid texture in swap chain!");

			// NOTE: we only destroy the RTV descriptor, everything else is handled by the swap chain object
			m_internal->m_nri_interface.core.DestroyDescriptor(current_texture_data.nri_rtv.descriptor.nri_descriptor);
			current_texture_data.nri_rtv.descriptor.nri_descriptor = nullptr;
			current_texture_data.window.invalidate();

			// Remove the texture entry
			m_internal->m_textures.remove(current_texture.texture);
		}

		swap_chain.textures.clear();

		m_internal->m_nri_interface.swap_chain.DestroySwapChain(swap_chain.nri_swapchain);
		swap_chain.nri_swapchain = nullptr;

		m_internal->m_windows.remove(window_handle);
	}

	void Device::wait_idle()
	{
		m_internal->m_nri_interface.core.DeviceWaitIdle(m_internal->m_device);
	}

	nri::CoreInterface& Device::get_nri_core_interface()
	{
		return m_internal->m_nri_interface.core;
	}

	nri::StreamerInterface& Device::get_nri_streamer_interface()
	{
		return m_internal->m_nri_interface.streamer;
	}

	bool Device::initialize()
	{
		if (m_internal->initialize(*this) == false)
		{
			return false;
		}

		return true;
	}

	void Device::shutdown()
	{
		m_internal->shutdown();
	}
}
