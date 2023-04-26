cbuffer ExternalData
{
    float2 mirrorMapDimensions;
};

Texture2D MirrorMap : register(t0);
SamplerState SamplerOptions : register(s0);

// Fill every pixel taken up by this mirror white.
// This is the culled version of this shader, meaning it is meant
// for use when rendering a new mirror THROUGH another mirror
float4 main(float4 input : SV_POSITION) : SV_TARGET
{
    if (MirrorMap.Sample(SamplerOptions, input.xy / mirrorMapDimensions).r < 1)
        discard;
    return float4(1, 1, 1, 1);
}