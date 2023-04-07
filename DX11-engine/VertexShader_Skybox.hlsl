struct VertexShaderInput
{
    float3 position : POSITION; // XYZ position
    float3 normal : NORMAL; // Normal
    float2 uv : TEXCOORD; // UV coordinate
    float3 tangent : TANGENT; // Tangent
};

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

VertexToPixel main(VertexShaderInput input)
{
    VertexToPixel output;
    
    // Create copy of view without translation data
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    output.position = mul(projection, mul(viewNoTranslation, float4(input.position, 1)));
    output.position.z = output.position.w; // make sure position depth is 1 after perspective divide
    output.sampleDir = input.position; // direction to skybox vertex is just its position
    
    return output;
}