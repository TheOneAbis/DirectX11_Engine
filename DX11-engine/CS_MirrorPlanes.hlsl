// This Compute Shader is used to calculate the positions and normals of the mirror's planes
// in world space, to be used in rendering through the mirrors to check if object pixels
// are inside the mirror, in which case we would not discard it.

struct Plane
{
    float3 Position;
    float3 Normal;
};

cbuffer ExternalData
{
    matrix mirrorWorld;
    matrix mirrorWorldInvTranspose;
};

RWStructuredBuffer<Plane> planes : register(u0);

// main
[numthreads(4, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    planes[DTid.x].Position = mul(mirrorWorld, float4(planes[DTid.x].Position, 1.0f));
    planes[DTid.x].Normal = mul((float3x3)mirrorWorldInvTranspose, planes[DTid.x].Normal);
}