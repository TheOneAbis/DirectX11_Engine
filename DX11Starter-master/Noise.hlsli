#ifndef __SHADER_NOISE_INCLUDE__
#define __SHADER_NOISE_INCLUDE__

// Implementation of Perlin Noise. 
// Source Article: https://rtouti.github.io/graphics/perlin-noise-algorithm
// Helper to get the constant vector for a corner
float2 GetConstantVector(int v)
{
		// v is the value from the permutation table
    const int h = v & 3; // equivalent to v % 4
    if (h == 0)
        return float2(1.0, 1.0);
    else if (h == 1)
        return float2(-1.0, 1.0);
    else if (h == 2)
        return float2(-1.0, -1.0);
    else
        return float2(1.0, -1.0);
}

// quick ease function (this is what Ken Perlin used)
float Fade(float t)
{
    return ((6.0f * t - 15.0f) * t + 10.0f) * t * t * t;
}

float Perlin2D(float x, float y, int wrap, int perlinTable[])
{
    const int X = (int) floor(x) & (wrap - 1);
    const int Y = (int) floor(y) & (wrap - 1);
    const float xf = x - (int) floor(x);
    const float yf = y - (int) floor(y);

	// Vectors pointing from grid points TO input point
    float2 topRight = { xf - 1.0f, yf - 1.0f };
    float2 topLeft = { xf, yf - 1.0f };
    float2 bottomRight = { xf - 1.0f, yf };
    float2 bottomLeft = { xf, yf };

	// Select a value in the array for each of the 4 corners
    const float2 vecTopRight = GetConstantVector(perlinTable[perlinTable[X + 1] + Y + 1]);
    const float2 vecTopLeft = GetConstantVector(perlinTable[perlinTable[X] + Y + 1]);
    const float2 vecBottomRight = GetConstantVector(perlinTable[perlinTable[X + 1] + Y]);
    const float2 vecBottomLeft = GetConstantVector(perlinTable[perlinTable[X] + Y]);

	// Calculate dot products of each corner
    float dotTopRight = dot(topRight, vecTopRight);
    float dotTopLeft = dot(topLeft, vecTopLeft);
    float dotBottomRight = dot(bottomRight, vecBottomRight);
    float dotBottomLeft = dot(bottomLeft, vecBottomLeft);

	// Interpolate x and y to get final value
    const float u = Fade(xf);
    const float v = Fade(yf);
    
    return lerp(
        lerp(dotBottomLeft, dotTopLeft, v),
        lerp(dotBottomRight, dotTopRight, v), u);
}

float FractalBrownianMotion(float x, float y, int numOctaves, int wrap, int perlinTable[])
{
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.005f;

    for (int octave = 0; octave < numOctaves; octave++)
    {
        result += amplitude * Perlin2D(x * frequency, y * frequency, wrap, perlinTable);

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return result;
}

#endif