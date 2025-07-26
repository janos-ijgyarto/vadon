struct VS_OUTPUT
{
    float4 screen_position : SV_POSITION;
    float2 uv : UV;
};

VS_OUTPUT vs_main(uint vertex_id : SV_VertexID)
{
    VS_OUTPUT output;

    // Generate triangle that fills screen using vertex ID
    output.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
    
    float2 screen_position = (output.uv * float2(2, -2)) + float2(-1, 1);
    output.screen_position = float4(screen_position, 0, 1);

    return output;
}

sampler main_sampler : register(s0);
Texture2D main_texture : register(t0);

#define PS_INPUT VS_OUTPUT

float4 ps_main(PS_INPUT input) : SV_Target
{
    return main_texture.Sample(main_sampler, input.uv);
}