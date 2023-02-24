cbuffer ExternalData : register(b0)
{
    float4 colorTint;
	float3 ambientColor;

	float3 lightDir;
	float3 lightColor;

	float3 light2Dir;
	float3 light2Color;

	float3 cameraPosition;
    float specularAmt;
}

float DiffuseBRDF(float3 normal, float3 dirToLight)
{
	// saturate(x) clamps a value between 0 and 1 (so it's a glorified clamp(x, 0, 1) basically)
	return saturate(dot(normal, dirToLight));
}

float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector, float specularAmt)
{
	// Get reflection of light bounsing off the surface 
	float3 refl = reflect(lightDir, normal);

	// Compare reflection against view vector, raising it to
	// a very high power to ensure the falloff to zero is quick
    return pow(saturate(dot(refl, viewVector)), 1 + saturate(specularAmt) * 255);
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float3 normal           : NORMAL;
	float2 uv               : TEXCOORD;
	float3 worldPosition    : POSITION;
};

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

	// Start our light total off with just ambient
	float3 totalLightColor = ambientColor;
    
	// Calculate light 1
    totalLightColor += DiffuseBRDF(input.normal, -lightDir) * lightColor * colorTint.xyz;
	totalLightColor += SpecularBRDF(input.normal, lightDir, viewVector, specularAmt) * lightColor;

	// Calculate light 2
	totalLightColor += DiffuseBRDF(input.normal, -light2Dir) * light2Color * colorTint.xyz;
    totalLightColor += SpecularBRDF(input.normal, light2Dir, viewVector, specularAmt) * light2Color;

    return float4(totalLightColor, 1);
}