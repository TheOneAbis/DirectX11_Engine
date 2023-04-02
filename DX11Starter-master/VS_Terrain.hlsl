#include "ShaderIncludes.hlsli"
#include "Noise.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix worldInvTranspose;
    matrix view;
    matrix projection;
    
    int4 permutation[128];
	
    float noiseOffset;
    float2 noiseDensity;
}

VertexToPixel main(VertexShaderInput input)
{
    // Unpack the permutation array (because cbuffers pad each individual 
    // element of an array to guarentee 16 bytes between each one)
    int permutationUnpacked[512];
    for (int i = 0; i < 512; i++)
        permutationUnpacked[i] = permutation[i / 4][i % 4];
	
	// Set up output struct
    VertexToPixel output;
    
    // Alter the y translation portion of world matrix w/ perlin noise
    input.localPosition.y += FractalBrownianMotion(
        input.localPosition.x * noiseDensity.x + noiseOffset, 
        input.localPosition.z * noiseDensity.y, 
        1, 
        permutationUnpacked);
	
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