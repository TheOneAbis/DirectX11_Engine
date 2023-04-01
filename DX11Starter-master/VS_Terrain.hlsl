#include "ShaderIncludes.hlsli"
#include "Noise.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix worldInvTranspose;
    matrix view;
    matrix projection;
    
    int perlinTable[512];
}

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;
	
	// Multiply the three matrices together first
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// world position of the vertex
    output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;

	// This normal is in LOCAL space, not WORLD space
	// To go from local -> world, we need a world matrix (specifically its rotation and scale components)
    output.normal = mul((float3x3) worldInvTranspose, input.normal);
    output.tangent = mul((float3x3) world, input.tangent);

	// Pass UV to PS
    output.uv = input.uv;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
    return output;
}