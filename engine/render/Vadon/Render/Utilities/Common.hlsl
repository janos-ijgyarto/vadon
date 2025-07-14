#ifndef VADON_RENDER_UTILITIES_COMMON_HLSL
#define VADON_RENDER_UTILITIES_COMMON_HLSL
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
#endif