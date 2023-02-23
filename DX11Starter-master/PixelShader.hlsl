cbuffer ExternalData : register(b0)
{
    float4 colorTint;
	float3 ambientColor;
	float3 lightDir;
	float3 lightColor;
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
	// Compare the light's direction and the surface direction
	// -lightDir = direction to light
	float shadingResult = dot(input.normal, -lightDir);

	float3 totalLightColor = ambientColor + lightColor * shadingResult;
	
    return float4(totalLightColor, 1) * colorTint;
}