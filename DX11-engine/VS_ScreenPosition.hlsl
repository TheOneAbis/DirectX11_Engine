#include "ShaderIncludes.hlsli"

// Make sure the data in here does not overlap a 16-byte boundary (1 float = 4 bytes)
cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexShaderInput input) : SV_POSITION
{
	// Only need screen position of vertices
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));
}