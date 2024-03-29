#include "ShaderIncludes.hlsli"
struct Plane
{
    float4 Position;
    float3 Normal;
};

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    
    float roughness;
    float3 cameraPosition;
    
    float metalness;
    int textureBitMask;
    float2 uvOffset;
    
    Light lights[MAX_LIGHT_COUNT];
    
    float textureScale;
    float3 ambient;
    
    float3 mirrorNormal;
    float2 mirrorMapDimensions;
    float3 mirrorPos;
    
    Plane mirrorPlanes[4];
}

Texture2D AlbedoMap : register(t0);    // Albedo map
Texture2D RoughnessMap : register(t1); // Roughness map
Texture2D NormalMap : register(t2);    // Normal map
Texture2D MetalnessMap : register(t3); // Metalness map
Texture2D MirrorMap : register(t4);    // mirror map that was drawn to

SamplerState SamplerOptions : register(s0);

// --------------------------------------------------------
// The entry point for pixel shader
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Skip this pixel if it does not overlap with the mirror's pixels OR it is behind the mirror
    // (the mirror pixels are white, everywhere else is black)
        if (MirrorMap.Sample(SamplerOptions, input.screenPosition.xy / mirrorMapDimensions).r < 1 ||
        dot(normalize(-mirrorNormal), normalize(input.worldPosition - mirrorPos)) <= 0)
            discard;
    
    // Renormalize normals and tangents
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    // scroll uv
    input.uv += uvOffset;

    // Calculate vector from surface to camera
    float3 viewVector = normalize(cameraPosition - input.worldPosition);

    // If we're using normal map
    if ((textureBitMask & BIT_NORMAL) == BIT_NORMAL)
    {
        // Renormalize from the map if using normal map
        float3 normalFromMap = normalize(NormalMap.Sample(SamplerOptions, input.uv * textureScale).rgb * 2 - 1); // scale 0 to 1 values to -1 to 1
        
        // rotate normal map to convert from tangent to world space (since our input values are already in world space from VS)
        // Ensure we orthonormalize the tangent again
        float3 N = normalize(input.normal); // Normal
        float3 T = normalize(input.tangent); // Tangent
        T = normalize(T - N * dot(T, N)); // Gram-Schmidt orthonomalizing of the Tangent
        float3 B = cross(T, N); // Bi-tangent
        float3x3 TBN = float3x3(T, B, N); // TBN rotation matrix

        // multiply normal map vector by the TBN matrix
        input.normal = mul(normalFromMap, TBN);
    }

    // Sample the surface texture for the initial pixel color (scale texture if a scale was specified)
    // If using texture for surface, un-correct the color w/ gamma value
    float3 albedoColor = ((textureBitMask & BIT_ALBEDO) == BIT_ALBEDO ? pow(AlbedoMap.Sample(SamplerOptions, input.uv * textureScale).rgb, 2.2f) : 1) * colorTint.xyz;
    float metal = (textureBitMask & BIT_METALNESS) == BIT_METALNESS ? MetalnessMap.Sample(SamplerOptions, input.uv * textureScale).r : metalness;
    float rough = (textureBitMask & BIT_ROUGHNESS) == BIT_ROUGHNESS ? RoughnessMap.Sample(SamplerOptions, input.uv * textureScale).r : roughness;
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(NONMETAL_F0, albedoColor.rgb, metal);

    float3 totalLightColor = ambient * albedoColor * (1 - metal);
    // Loop through the lights
    for (uint i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        float3 lightDir;
        bool attenuate = false;
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
        float3 lightCol = ColorFromLightPBR(input.normal, -lightDir, lights[i].Color,
            albedoColor, viewVector, lights[i].Intensity, rough, metal, specularColor);

        // If this is a point or spot light, attenuate the color
        if (attenuate)
            lightCol *= Attenuate(lights[i], input.worldPosition);
        
        totalLightColor += lightCol;
    }
    
    // correct color w/ gamma and return final color
    return float4(pow(totalLightColor, 1.0f / 2.2f), 1);
}