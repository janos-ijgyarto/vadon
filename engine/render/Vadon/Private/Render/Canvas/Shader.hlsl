#ifndef MAX_LAYER_COUNT
#define MAX_LAYER_COUNT 4096
#endif

static const uint c_primitive_type_triangle = 0;
static const uint c_primitive_type_rectangle_fill = 1;
static const uint c_primitive_type_rectangle_outline = 2;

static const uint c_layer_index_mask = (1 << 24) - 1;

struct PrimitiveInfo
{
    uint type;
    uint layer;
};

PrimitiveInfo create_primitive_info(uint data)
{
    PrimitiveInfo info;
    info.type = data >> 24;
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

#define PRIMITIVE_TYPE_TRIANGLE 0
#define PRIMITIVE_TYPE_RECTANGLE_FILL 1
#define PRIMITIVE_TYPE_RECTANGLE_OUTLINE 2

struct PrimitiveVertex
{ 
    float2 position;
    float2 uv;
};

struct PrimitiveRectangle
{
    float2 position;
    float2 size;    
};

struct TrianglePrimitiveData
{
    uint info;
    uint material;
    float depth;
    uint _padding;
    
    PrimitiveVertex vertices[3];

    float4 color;
};

struct RectanglePrimitiveData
{
    uint info;
    uint material;
    float depth;
    float thickness;
    
    PrimitiveRectangle dimensions;
    
    PrimitiveRectangle uv_dimensions;
    
    float4 color;
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
    const float4 data = primitive_buffer[get_primitive_data_offset(index)];
    return asuint(data.y);
}

TrianglePrimitiveData unpack_triangle_data(uint vertex_index)
{
    TrianglePrimitiveData triangle_data;

    // Extract all the parts from the buffer
    const uint triangle_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 triangle_data0 = primitive_buffer[triangle_data_offset];
    triangle_data.info = asuint(triangle_data0.x);
    triangle_data.material = asuint(triangle_data0.y);
    triangle_data.depth = triangle_data0.z;
    triangle_data._padding = asuint(triangle_data0.w);
    
    for (int i = 0; i < 3; ++i)
    {
        const float4 triangle_vertex_data = primitive_buffer[triangle_data_offset + 1 + i];
        triangle_data.vertices[i].position = triangle_vertex_data.xy;
        triangle_data.vertices[i].uv = triangle_vertex_data.zw;
    }
    
    triangle_data.color = primitive_buffer[triangle_data_offset + 4];
    
    return triangle_data;
}

PS_INPUT get_triangle_vertex(uint vertex_index)
{
    const TrianglePrimitiveData triangle_data = unpack_triangle_data(vertex_index);
    const uint triangle_vertex_index = get_primitive_vertex_index(vertex_index);
    
    PS_INPUT output;
    output.pos = float4(triangle_data.vertices[triangle_vertex_index].position, triangle_data.depth, 1);
    output.uv = float3(triangle_data.vertices[triangle_vertex_index].uv, 0);
    output.col = triangle_data.color;
    
    return output;
}

RectanglePrimitiveData unpack_rectangle_data(uint vertex_index)
{
    RectanglePrimitiveData rectangle_data;
    
    // Extract all the parts from the buffer
    const uint rectangle_data_offset = get_primitive_data_offset(vertex_index);
    
    const float4 rectangle_data0 = primitive_buffer[rectangle_data_offset];
    rectangle_data.info = asuint(rectangle_data0.x);
    rectangle_data.material = asuint(rectangle_data0.y);
    rectangle_data.depth = rectangle_data0.z;
    rectangle_data.thickness = rectangle_data0.w;
    
    const float4 rectangle_data1 = primitive_buffer[rectangle_data_offset + 1];
    rectangle_data.dimensions.position = rectangle_data1.xy;
    rectangle_data.dimensions.size = rectangle_data1.zw;
    
    const float4 rectangle_data2 = primitive_buffer[rectangle_data_offset + 2];
    rectangle_data.uv_dimensions.position = rectangle_data2.xy;
    rectangle_data.uv_dimensions.size = rectangle_data2.zw;
    
    rectangle_data.color = primitive_buffer[rectangle_data_offset + 3];

    return rectangle_data;
}

static const float2 c_rectangle_offsets[] = { float2(-1, 1), float2(1, 1), float2(-1, -1), float2(1, -1) };
static const float2 c_rectangle_uv_offsets[] = { float2(0, 0), float2(1, 0), float2(0, 1), float2(1, 1) };

PS_INPUT get_rectangle_fill_vertex(uint vertex_index)
{
    const RectanglePrimitiveData rectangle_data = unpack_rectangle_data(vertex_index);
    const uint corner_index = get_primitive_vertex_index(vertex_index);
    
    const float2 corner_position = rectangle_data.dimensions.position + (rectangle_data.dimensions.size * 0.5f * c_rectangle_offsets[corner_index]);
    
    // FIXME: change this so sprites can also be rotated, mirrored, etc.
    const float2 corner_uv = rectangle_data.uv_dimensions.position + (rectangle_data.uv_dimensions.size * c_rectangle_uv_offsets[corner_index]);
    
    PS_INPUT output;
    output.pos = float4(corner_position, rectangle_data.depth, 1);
    output.uv = corner_uv;
    output.col = rectangle_data.color;
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
    output.pos = float4(corner_position, rectangle_data.depth, 1);
    output.uv = float3(0, 0, 0); // FIXME: allow texturing?
    output.col = rectangle_data.color;
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