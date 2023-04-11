struct VertexToPixel
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

cbuffer ExternalData
{
    float2 mirrorMapDimensions;
};

TextureCube SkyboxTexture : register(t0);
Texture2D MirrorMap : register(t1);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Discard the pixel if it's not in the mirror
    if (MirrorMap.Sample(BasicSampler, input.position.xy / mirrorMapDimensions).r == 0)
        discard;

    return SkyboxTexture.Sample(BasicSampler, input.sampleDir);
}