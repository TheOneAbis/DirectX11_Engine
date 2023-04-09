// For mirrors, the render target should be a texture w/ same dimensions as viewport.
// Just need every pixel the mirror takes up.
float4 main() : SV_TARGET
{
    return float4(1, 1, 1, 1);
}