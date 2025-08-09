#include <Vadon/Render/Utilities/Common.hlsl>

#ifndef MAX_LAYER_COUNT
#define MAX_LAYER_COUNT 4096
#endif

static const uint c_primitive_type_triangle = 0;
static const uint c_primitive_type_rectangle_fill = 1;
static const uint c_primitive_type_rectangle_outline = 2;
static const uint c_primitive_type_circle = 3;
static const uint c_primitive_type_sprite = 4;

static const uint c_layer_index_width = 8;
static const uint c_material_index_width = 16;

static const uint c_layer_index_mask = (1 << c_layer_index_width) - 1;
static const uint c_material_index_mask = (1 << c_material_index_width) - 1;

struct PrimitiveInfo
{
    uint type;
    uint layer;
    uint material;
};

PrimitiveInfo create_primitive_info(uint data)
{
    PrimitiveInfo info;
    info.type = data >> (c_layer_index_width + c_material_index_width);
    info.material = (data >> c_layer_index_width) & c_material_index_mask;
    info.layer = data & c_layer_index_mask;
    
    return info;
}

static const uint c_layer_view_agnostic = 1 << 0;

struct LayerData
{
    float2 offset;
    float scale;
    uint flags;
};

static const uint c_sdf_flags_enabled = 1 << 0;

struct SDF
{
    uint flags;
    float edge_distance;
    float edge_width_factor;
    uint _padding;
};

cbuffer View : register(b0)
{
    struct
    {        
        float4x4 view_projection;
        float4x4 projection;
    } View;
}

cbuffer LayerBuffer : register(b1)
{
    struct
    {
        LayerData layers[MAX_LAYER_COUNT];
    } LayerBuffer;
}

struct Material
{
    SDF sdf;
};

StructuredBuffer<Material> material_buffer : register(t0);
StructuredBuffer<float4> primitive_buffer : register(t1);

sampler main_sampler : register(s2);
Texture2D main_texture : register(t2);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    nointerpolation uint info : PRIMITIVE_INFO;
};

#define PRIMITIVE_INDEX_DATA_OFFSET_WIDTH 24
#define PRIMITIVE_OFFSET_MASK ((1 << PRIMITIVE_INDEX_DATA_OFFSET_WIDTH) - 1);

struct PrimitiveVertex
{ 
    float2 position;
    float2 uv;
};

struct TrianglePrimitiveData
{
    uint info;
    uint3 color;
    PrimitiveVertex points[3];
};

struct PrimitiveRectangle
{
    float2 position;
    float2 size;    
};

struct RectanglePrimitiveData
{
    uint info;
    uint color;
    float thickness;
    uint _padding;

    PrimitiveRectangle dimensions;
};

struct CirclePrimitiveData
{
    uint info;
    float2 position;
    uint color;
    float radius;
    float3 _padding; // TODO: use for extra info?
    PrimitiveRectangle uv_dimensions;
};

struct SpritePrimitiveData
{
    uint info;
    uint color;
    uint2 _padding;

    PrimitiveRectangle dimensions;
    PrimitiveRectangle uv_dimensions;
};

uint get_primitive_data_offset(uint index)
{
    return index & PRIMITIVE_OFFSET_MASK;
}

uint get_primitive_info_uint(uint index)
{
    const float4 data = primitive_buffer[get_primitive_data_offset(index)];
    return asuint(data.x);
}

uint get_primitive_vertex_index(uint index)
{
    return (index >> PRIMITIVE_INDEX_DATA_OFFSET_WIDTH);
}

TrianglePrimitiveData unpack_triangle_data(uint vertex_index)
{
    TrianglePrimitiveData triangle_data;

    // Extract all the parts from the buffer
    const uint triangle_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 triangle_data0 = primitive_buffer[triangle_data_offset];
    triangle_data.info = asuint(triangle_data0.x);
    triangle_data.color = asuint(triangle_data0.yzw);
    
    for (int i = 0; i < 3; ++i)
    {
        const float4 triangle_vertex_data = primitive_buffer[triangle_data_offset + 1 + i];
        PrimitiveVertex current_vertex;
        current_vertex.position = triangle_vertex_data.xy;
        current_vertex.uv = triangle_vertex_data.zw;
        
        triangle_data.points[i] = current_vertex;
    }
    
    return triangle_data;
}

void get_triangle_vertex(uint vertex_index, out PS_INPUT output)
{
    const TrianglePrimitiveData triangle_data = unpack_triangle_data(vertex_index);
    const uint triangle_vertex_index = get_primitive_vertex_index(vertex_index);
    
    output.pos = float4(triangle_data.points[triangle_vertex_index].position, 0, 1);
    output.col = decode_rgba_uint(triangle_data.color[triangle_vertex_index]);
    output.uv0 = float2(triangle_data.points[triangle_vertex_index].uv);
    output.uv1 = float2(0, 0);
    output.info = 0;
}

RectanglePrimitiveData unpack_rectangle_data(uint vertex_index)
{
    RectanglePrimitiveData rectangle_data;
    
    // Extract all the parts from the buffer
    const uint rectangle_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 rectangle_data0 = primitive_buffer[rectangle_data_offset];
    rectangle_data.info = asuint(rectangle_data0.x);
    rectangle_data.color = asuint(rectangle_data0.y);
    rectangle_data.thickness = rectangle_data0.z;
    rectangle_data._padding = asuint(rectangle_data0.w);
    
    const float4 rectangle_data1 = primitive_buffer[rectangle_data_offset + 1];
    rectangle_data.dimensions.position = rectangle_data1.xy;
    rectangle_data.dimensions.size = rectangle_data1.zw;

    return rectangle_data;
}

CirclePrimitiveData unpack_circle_data(uint vertex_index)
{
    CirclePrimitiveData circle_data;
    
    // Extract all the parts from the buffer
    const uint circle_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 circle_data0 = primitive_buffer[circle_data_offset];
    circle_data.info = asuint(circle_data0.x);
    circle_data.position = circle_data0.yz;
    circle_data.color = asuint(circle_data0.w);
    
    const float4 circle_data1 = primitive_buffer[circle_data_offset + 1];
    circle_data.radius = circle_data1.x;
    circle_data._padding = circle_data1.yzw;
        
    const float4 circle_data2 = primitive_buffer[circle_data_offset + 2];
    circle_data.uv_dimensions.position = circle_data2.xy;
    circle_data.uv_dimensions.size = circle_data2.zw;

    return circle_data;
}

SpritePrimitiveData unpack_sprite_data(uint vertex_index)
{
    SpritePrimitiveData sprite_data;
    
    // Extract all the parts from the buffer
    const uint sprite_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 sprite_data0 = primitive_buffer[sprite_data_offset];
    sprite_data.info = asuint(sprite_data0.x);
    sprite_data.color = asuint(sprite_data0.y);
    sprite_data._padding = asuint(sprite_data0.zw);
    
    const float4 sprite_data1 = primitive_buffer[sprite_data_offset + 1];
    sprite_data.dimensions.position = sprite_data1.xy;
    sprite_data.dimensions.size = sprite_data1.zw;
    
    const float4 sprite_data2 = primitive_buffer[sprite_data_offset + 2];
    sprite_data.uv_dimensions.position = sprite_data2.xy;
    sprite_data.uv_dimensions.size = sprite_data2.zw;

    return sprite_data;
}

static const float2 c_rectangle_offsets[] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
static const float2 c_rectangle_uv_offsets[] = { float2(0, 0), float2(1, 0), float2(0, 1), float2(1, 1) };

void get_rectangle_fill_vertex(uint vertex_index, out PS_INPUT output)
{
    const RectanglePrimitiveData rectangle_data = unpack_rectangle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    // TODO: add logic so sprites can also be rotated, mirrored, etc.
    const float2 corner_position = rectangle_data.dimensions.position + (rectangle_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index]);
    
    output.pos = float4(corner_position, 0, 1);
    output.col = decode_rgba_uint(rectangle_data.color);
    output.uv0 = float2(0, 0);
    output.uv1 = float2(0, 0);
    output.info = 0;
}

static const float2 c_rectangle_thickness_offsets[] = { float2(0, 0), float2(0, 0), float2(0, 0), float2(0, 0), float2(1, -1), float2(-1, -1), float2(1, 1), float2(-1, 1) };

void get_rectangle_outline_vertex(uint vertex_index, out PS_INPUT output)
{
    const RectanglePrimitiveData rectangle_data = unpack_rectangle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    float2 corner_position = rectangle_data.dimensions.position + (rectangle_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index % 4]);
    corner_position += c_rectangle_thickness_offsets[corner_index] * rectangle_data.thickness;
    
    output.pos = float4(corner_position, 0, 1);
    output.col = decode_rgba_uint(rectangle_data.color);
    output.uv0 = float2(0, 0);
    output.uv1 = float2(0, 0);
    output.info = 0;
}

void get_sprite_vertex(uint vertex_index, out PS_INPUT output)
{
    const SpritePrimitiveData sprite_data = unpack_sprite_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    // TODO: add logic so sprites can also be rotated, mirrored, etc.
    const float2 corner_position = sprite_data.dimensions.position + (sprite_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index]);
    const float2 corner_uv = sprite_data.uv_dimensions.position + (sprite_data.uv_dimensions.size * c_rectangle_uv_offsets[corner_index]);
    
    output.pos = float4(corner_position, 0, 1);
    output.col = decode_rgba_uint(sprite_data.color);
    output.uv0 = corner_uv;
    output.uv1 = float2(0, 0);
    output.info = 0;
}

void get_circle_vertex(uint vertex_index, out PS_INPUT output)
{
    // NOTE: essentially same as sprite, but we need to compute the corners from circle params
    const CirclePrimitiveData circle_data = unpack_circle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
     // TODO: add logic so sprites can also be rotated, mirrored, etc.
    const float2 corner_position = circle_data.position + (circle_data.radius * c_rectangle_offsets[corner_index]);
    const float2 corner_uv = circle_data.uv_dimensions.position + (circle_data.uv_dimensions.size * c_rectangle_uv_offsets[corner_index]);
    
    output.pos = float4(corner_position, 0, 1);
    output.col = decode_rgba_uint(circle_data.color);
    output.uv0 = corner_uv;
    output.uv1 = c_rectangle_uv_offsets[corner_index]; // Use uv1 to determine where we are in the quad
    output.info = 0;
}

PS_INPUT vs_main(uint vertex_index : SV_VertexID)
{
    PS_INPUT output;
    uint primitive_info_value = get_primitive_info_uint(vertex_index);
    
    const PrimitiveInfo primitive_info = create_primitive_info(primitive_info_value);
    
    switch (primitive_info.type)
    {
        case c_primitive_type_triangle:
            get_triangle_vertex(vertex_index, output);
            break;
        case c_primitive_type_rectangle_fill:
            get_rectangle_fill_vertex(vertex_index, output);
            break;
        case c_primitive_type_rectangle_outline:
            get_rectangle_outline_vertex(vertex_index, output);
            break;
        case c_primitive_type_circle:
            get_circle_vertex(vertex_index, output);
            break;
        case c_primitive_type_sprite:
            get_sprite_vertex(vertex_index, output);
            break;
        default:
            {
                output.pos = float4(0, 0, 0, 1);
                output.col = float4(0, 0, 0, 0);
                output.uv0 = float2(0, 0);
                output.uv1 = float2(0, 0);
            }
            break;
    }
    
    const LayerData layer_data = LayerBuffer.layers[primitive_info.layer];
    
    output.pos.xy += layer_data.offset;
    output.pos.xy *= layer_data.scale;
    if(layer_data.flags & c_layer_view_agnostic)
    {
        output.pos.xy = mul(output.pos, View.projection).xy;
    }
    else
    {
        output.pos.xy = mul(output.pos, View.view_projection).xy;
    }
    output.info = primitive_info_value;
    return output;
}

float4 get_sdf_color(PS_INPUT input, Material material)
{
    // FIXME: should we just drop the pixel altogether if the SDF alpha value is too low?;
    const float distance = main_texture.Sample(main_sampler, input.uv0).x;
    const float edge_width = material.sdf.edge_width_factor * fwidth(distance);
    
    const float opacity = smoothstep(material.sdf.edge_distance - edge_width, material.sdf.edge_distance + edge_width, distance);
    
    return float4(input.col.xyz, input.col.w * opacity);
}

float4 ps_main(PS_INPUT input) : SV_Target
{
    const PrimitiveInfo primitive_info = create_primitive_info(input.info);
    
    if (primitive_info.type == c_primitive_type_circle)
    {
        const float2 center_distance = input.uv1 - float2(0.5f, 0.5f);
        if (dot(center_distance, center_distance) > 0.25f)
        {
            discard;
        }
    }
    
    const Material material = material_buffer[primitive_info.material];
    float4 result = (material.sdf.flags & c_sdf_flags_enabled) ? get_sdf_color(input, material) : (input.col * main_texture.Sample(main_sampler, input.uv0.xy));
    
    // Discard pixel if alpha is under certain value
    // FIXME: should be configurable?
    if (result.a < 0.001f)
    {
        discard;
    }
    
    return result;
}