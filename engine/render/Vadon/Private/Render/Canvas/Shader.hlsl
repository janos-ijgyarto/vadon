#ifndef MAX_LAYER_COUNT
#define MAX_LAYER_COUNT 4096
#endif

static const uint c_primitive_type_triangle = 0;
static const uint c_primitive_type_rectangle_fill = 1;
static const uint c_primitive_type_rectangle_outline = 2;
static const uint c_primitive_type_sprite = 3;

static const uint c_layer_index_width = 8;
static const uint c_material_index_width = 16;

static const uint c_layer_index_mask = (1 << c_layer_index_width) - 1;
static const uint c_material_index_mask = (1 << c_material_index_width) - 1;

float4 decode_rgba_uint(uint color_uint)
{
    float4 color;
    const float norm = 1.0f / 255.0f;
    color.r = ((float) (color_uint & 0xFF)) * norm;
    color.g = ((float) ((color_uint >> 8) & 0xFF)) * norm;
    color.b = ((float) ((color_uint >> 16) & 0xFF)) * norm;
    color.a = ((float) ((color_uint >> 24) & 0xFF)) * norm;
    
    return color;
}

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
    float2 uv : TEXCOORD0;
    float4 col : COLOR0;
    uint primitive : PRIMITIVE_INDEX; // FIXME: could instead use the SV_Primitive
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

PrimitiveInfo get_primitive_info(uint index)
{
    const float4 data = primitive_buffer[get_primitive_data_offset(index)];
    return create_primitive_info(asuint(data.x));
}

uint get_primitive_vertex_index(uint index)
{
    return (index >> PRIMITIVE_INDEX_DATA_OFFSET_WIDTH);
}

uint get_material_index(uint index)
{
    const PrimitiveInfo primitive_info = get_primitive_info(index);
    return primitive_info.material;
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
        triangle_data.points[i].position = triangle_vertex_data.xy;
        triangle_data.points[i].uv = triangle_vertex_data.zw;
    }
    
    return triangle_data;
}

PS_INPUT get_triangle_vertex(uint vertex_index)
{
    const TrianglePrimitiveData triangle_data = unpack_triangle_data(vertex_index);
    const uint triangle_vertex_index = get_primitive_vertex_index(vertex_index);
    
    PS_INPUT output;
    output.pos = float4(triangle_data.points[triangle_vertex_index].position, 0, 1);
    output.uv = float3(triangle_data.points[triangle_vertex_index].uv, 0);
    output.col = decode_rgba_uint(triangle_data.color[triangle_vertex_index]);
    
    return output;
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

PS_INPUT get_rectangle_fill_vertex(uint vertex_index)
{
    const RectanglePrimitiveData rectangle_data = unpack_rectangle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    // TODO: add logic so sprites can also be rotated, mirrored, etc.
    const float2 corner_position = rectangle_data.dimensions.position + (rectangle_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index]);
    
    PS_INPUT output;
    output.pos = float4(corner_position, 0, 1);
    output.uv = float2(0, 0);
    output.col = decode_rgba_uint(rectangle_data.color);
    return output;
}

static const float2 c_rectangle_thickness_offsets[] = { float2(0, 0), float2(0, 0), float2(0, 0), float2(0, 0), float2(1, -1), float2(-1, -1), float2(1, 1), float2(-1, 1) };

PS_INPUT get_rectangle_outline_vertex(uint vertex_index)
{
    const RectanglePrimitiveData rectangle_data = unpack_rectangle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    float2 corner_position = rectangle_data.dimensions.position + (rectangle_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index % 4]);
    corner_position += c_rectangle_thickness_offsets[corner_index] * rectangle_data.thickness;
    
    PS_INPUT output;
    output.pos = float4(corner_position, 0, 1);
    output.uv = float2(0, 0);
    output.col = decode_rgba_uint(rectangle_data.color);
    return output;
}

PS_INPUT get_sprite_vertex(uint vertex_index)
{
    const SpritePrimitiveData sprite_data = unpack_sprite_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    // TODO: add logic so sprites can also be rotated, mirrored, etc.
    const float2 corner_position = sprite_data.dimensions.position + (sprite_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index]);
    const float2 corner_uv = sprite_data.uv_dimensions.position + (sprite_data.uv_dimensions.size * c_rectangle_uv_offsets[corner_index]);
    
    PS_INPUT output;
    output.pos = float4(corner_position, 0, 1);
    output.uv = corner_uv;
    output.col = decode_rgba_uint(sprite_data.color);
    return output;
}

PS_INPUT vs_main(uint vertex_index : SV_VertexID)
{
    const PrimitiveInfo primitive_info = get_primitive_info(vertex_index);
    
    PS_INPUT output;
    switch (primitive_info.type)
    {
        case c_primitive_type_triangle:
            output = get_triangle_vertex(vertex_index);
            break;
        case c_primitive_type_rectangle_fill:
            output = get_rectangle_fill_vertex(vertex_index);
            break;
        case c_primitive_type_rectangle_outline:
            output = get_rectangle_outline_vertex(vertex_index);
            break;
        case c_primitive_type_sprite:
            output = get_sprite_vertex(vertex_index);
            break;
        default:
            {
                output.pos = float4(0, 0, 0, 1);
                output.uv = float3(0, 0, 0);
                output.col = float4(0, 0, 0, 0);
            }
            break;
    }
    
    const LayerData layer_data = LayerBuffer.layers[primitive_info.layer];
    
    output.pos.xy += layer_data.offset;
    output.pos.xy *= layer_data.scale;
    if(layer_data.flags & c_layer_view_agnostic)
    {
        output.pos.xy = mul(output.pos, View.projection);
    }
    else
    {
        output.pos = mul(output.pos, View.view_projection);
    }
    output.primitive = vertex_index;
    return output;
}

float4 get_sdf_color(PS_INPUT input, Material material)
{
    // FIXME: should we just drop the pixel altogether if the SDF alpha value is too low?;
    const float distance = main_texture.Sample(main_sampler, input.uv);
    const float edge_width = material.sdf.edge_width_factor * fwidth(distance);
    
    const float opacity = smoothstep(material.sdf.edge_distance - edge_width, material.sdf.edge_distance + edge_width, distance);
    
    return float4(input.col.xyz, input.col.w * opacity);
}

float4 ps_main(PS_INPUT input) : SV_Target
{
    const Material material = material_buffer[get_material_index(input.primitive)];
    float4 result = (material.sdf.flags & c_sdf_flags_enabled) ? get_sdf_color(input, material) : (input.col * main_texture.Sample(main_sampler, input.uv.xy));
    
    // Discard pixel if alpha is under certain value
    // FIXME: should be configurable?
    if (result.a < 0.001f)
    {
        discard;
    }
    
    return result;
}