#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
    float3 ambientColor;
    float textureScale;
    float4 colorTint;
    float roughness;
	float3 cameraPosition;

    bool usesTextures;
    
    Light lights[MAX_LIGHT_COUNT];
}

Texture2D SurfaceTexture  : register(t0); // "t" registers for textures
Texture2D SpecularTexture  : register(t1); // Specular map
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
	// MUST re-normalize any interpolated vectors (rasterizer interpolates individual floats)
    input.normal = normalize(input.normal);
    
    // Calculate vector from surface to camera
    float3 viewVector = normalize(cameraPosition - input.worldPosition);

    // Sample the surface texture for the initial pixel color (scale texture if a scale was specified)
    float3 surfaceColor = (usesTextures ? SurfaceTexture.Sample(BasicSampler, input.uv * textureScale).rgb : 1) * colorTint.xyz;
    float3 totalLightColor = ambientColor * surfaceColor;
    
    float3 lightDir;
    bool attenuate = false;
    
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
            usesTextures ? SpecularTexture.Sample(BasicSampler, input.uv * textureScale).r : 1) * lights[i].Intensity;

        // If this is a point or spot light, attenuate the color
        if (attenuate)
            lightCol *= Attenuate(lights[i], input.worldPosition);
        
        totalLightColor += lightCol;
    }
    
    return float4(totalLightColor, 1);
}