#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 ambientColor;
    
    float2 mousePos;
    float time;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Cool mouse over effect thing
    float mouseOverEffect = clamp(distance(input.screenPosition.xy, mousePos) / 100.0f, 0, 1);
    
    // Apply this effect to area close to mouse
    float tanFunc = 0.5f + tan((input.uv.x * 10) + time * 5) / 10;
    float negTanFunc = 0.5f + tan((-input.uv.x * 10) + time * 3) / 10;
    float secFunc = 0.5f + 1/cos((input.uv.x * 5) + time * 3) / 10;
    
    float tanOffset = (abs(input.uv.y - tanFunc) * 8);
    float negTanOffset = (abs(input.uv.y - negTanFunc) * 8);
    float secOffset = (abs(input.uv.y - secFunc) * 5);
    
    float4 shadingNoHover = float4(tanOffset, negTanOffset, secOffset, 1);
    
    return lerp(shadingNoHover, float4(input.normal, 1), mouseOverEffect);
}