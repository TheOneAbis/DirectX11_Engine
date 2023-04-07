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
    for (uint i = 0; i < 512; i++)
        permutationUnpacked[i] = permutation[i / uint(4)][i % uint(4)];
	
	// Set up output struct
    VertexToPixel output;
    float s = noiseOffset / 100.0f;
    // Alter the y w/ fractal perlin noise (only 1 octave, can be expensive if increased)
    input.localPosition.y += Perlin2D(
        input.localPosition.x * noiseDensity.x + s,
        input.localPosition.z * noiseDensity.y, 
        //1, 
        permutationUnpacked);
    
    // ugly AF way to calculate normals
    float3 leftNeighbor = float3(input.localPosition.x - 1,
        input.localPosition.y + Perlin2D(
            (input.localPosition.x - 1) * noiseDensity.x + s,
            input.localPosition.z * noiseDensity.y,
           // 1,
            permutationUnpacked),
        input.localPosition.z) - input.localPosition;
    
    float3 upNeighbor = float3(input.localPosition.x,
        input.localPosition.y + Perlin2D(
            (input.localPosition.x) * noiseDensity.x + s,
            (input.localPosition.z + 1) * noiseDensity.y,
          //  1,
            permutationUnpacked),
        input.localPosition.z + 1) - input.localPosition;
    
    float3 rightNeighbor = float3(input.localPosition.x + 1,
        input.localPosition.y + Perlin2D(
            (input.localPosition.x + 1) * noiseDensity.x + s,
            input.localPosition.z * noiseDensity.y,
           // 1,
            permutationUnpacked),
        input.localPosition.z) - input.localPosition;
    
    float3 downNeighbor = float3(input.localPosition.x,
        input.localPosition.y + Perlin2D(
            (input.localPosition.x) * noiseDensity.x + s,
            (input.localPosition.z - 1) * noiseDensity.y,
            //1,
            permutationUnpacked),
        input.localPosition.z - 1) - input.localPosition;
    
    float3 newNormal = normalize(cross(leftNeighbor, upNeighbor) +
                       cross(upNeighbor, rightNeighbor) +
                       cross(rightNeighbor, downNeighbor) +
                       cross(downNeighbor, leftNeighbor));
	
	// Multiply the three matrices together first
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// world position of the vertex
    output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;

	// This normal is in LOCAL space, not WORLD space
    output.normal = mul((float3x3) worldInvTranspose, newNormal);
    output.tangent = mul((float3x3) world, input.tangent);

	// Pass UV to PS
    output.uv = input.uv;
    
    return output;
}