#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition :   POSITION; // XYZ position
    float3 normal :          NORMAL; // Normal
    float2 uv :              TEXCOORD; // UV coordinate
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name             Semantic
	//  |    |                   |
	//  v    v                   v
    float4 screenPosition :   SV_POSITION; // XYZW position (System Value Position)
    float3 normal :           NORMAL; // Normal
    float2 uv :               TEXCOORD;
    float3 worldPosition :    POSITION;
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f
#define MAX_LIGHT_COUNT 10

struct Light
{
    int Type;          // Which kind of light? 0, 1 or 2 (see above)
    float3 Direction;  // Directional and Spot lights need a direction
    float Range;       // Point and Spot lights have a max range for attenuation
    float3 Position;   // Point and Spot lights have a position in space
    float Intensity;   // All lights need an intensity
    float3 Color;      // All lights need a color
    float SpotFalloff; // Spot lights need a value to define their “cone” size
    float3 Padding;    // Purposefully padding to hit the 16-byte boundar	
};

float DiffuseBRDF(float3 normal, float3 dirToLight)
{
	// saturate(x) clamps a value between 0 and 1 (so it's a glorified clamp(x, 0, 1) basically)
    return saturate(dot(normal, dirToLight));
}

float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector, float roughness, float roughnessScale)
{
	// Get reflection of light bounsing off the surface 
    float3 refl = reflect(lightDir, normal);
    
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;

	// Compare reflection against view vector, raising it to
	// a very high power to ensure the falloff to zero is quick
    return pow(saturate(dot(refl, viewVector)), specExponent) * roughnessScale;
}

float3 ColorFromLight(float3 normal, float3 lightDir, float3 lightColor, float3 colorTint, float3 viewVec, float roughness, float roughnessScale)
{
    // Calculate diffuse an specular values
    float diffuse = DiffuseBRDF(normal, -lightDir);
    float spec = SpecularBRDF(normal, lightDir, viewVec, roughness, roughnessScale);

    return lightColor * colorTint * (diffuse + spec);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

#endif
