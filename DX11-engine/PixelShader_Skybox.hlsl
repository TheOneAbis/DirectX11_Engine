struct VertexToPixel
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

TextureCube SkyboxTexture : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    return SkyboxTexture.Sample(BasicSampler, input.sampleDir);
}