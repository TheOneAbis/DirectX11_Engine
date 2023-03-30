#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float3 ambientColor;
    float textureScale;
    float4 colorTint;
    float roughness;
	float3 cameraPosition;

    int textureBitMask;
    
    Light lights[MAX_LIGHT_COUNT];
}

Texture2D AlbedoMap : register(t0); // "t" registers for textures
Texture2D SpecularMap : register(t1); // Specular map
Texture2D NormalMap       : register(t2); // Normal map

SamplerState BasicSampler : register(s0); // "s" registers for samplers

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
    // Renormalize normals and tangents
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);

    // Calculate vector from surface to camera
    float3 viewVector = normalize(cameraPosition - input.worldPosition);

    // If we're using normal map
    if ((textureBitMask & 4) == 4)
    {
        // Renormalize from the map if using normal map
        float3 normalFromMap = normalize(NormalMap.Sample(BasicSampler, input.uv * textureScale).rgb * 2 - 1); // scale 0 to 1 values to -1 to 1
        
        // rotate normal map to convert from tangent to world space (since our input values are already in world space from VS)
        // Ensure we orthonormalize the tangent again
        float3 N = normalize(input.normal);  // Normal
        float3 T = normalize(input.tangent); // Tangent
        T = normalize(T - N * dot(T, N));    // Gram-Schmidt orthonomalizing of the Tangent
        float3 B = cross(T, N);              // Bi-tangent
        float3x3 TBN = float3x3(T, B, N);    // TBN rotation matrix

        // multiply normal map vector by the TBN matrix
        input.normal = mul(normalFromMap, TBN);
    }

    // Sample the surface texture for the initial pixel color (scale texture if a scale was specified)
    float3 surfaceColor = ((textureBitMask & 1) == 1 ? AlbedoMap.Sample(BasicSampler, input.uv * textureScale).rgb : 1) * colorTint.xyz;
    float3 totalLightColor = ambientColor * surfaceColor;
    
    bool attenuate = false;
    float3 lightDir;
    // Loop through the lights
    for (uint i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                lightDir = normalize(lights[i].Direction);
                break;
            case LIGHT_TYPE_POINT:
                lightDir = normalize(input.worldPosition - lights[i].Position);
                attenuate = true;
                break;
            case LIGHT_TYPE_SPOT:
                lightDir = normalize(input.worldPosition - lights[i].Position);
                attenuate = true;
                break;
        }
        float3 lightCol = ColorFromLight(
            input.normal,
            lightDir,
            lights[i].Color,
            surfaceColor,
            viewVector,
            roughness,
            (textureBitMask & 2) == 2 ? SpecularMap.Sample(BasicSampler, input.uv * textureScale).r : 1) * lights[i].Intensity;

        // If this is a point or spot light, attenuate the color
        if (attenuate)
            lightCol *= Attenuate(lights[i], input.worldPosition);
        
        totalLightColor += lightCol;
    }
    
    return float4(totalLightColor, 1);
}