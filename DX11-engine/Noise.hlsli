#ifndef __SHADER_NOISE_INCLUDE__
#define __SHADER_NOISE_INCLUDE__

// Implementation of Perlin Noise. 
// Source Article: https://rtouti.github.io/graphics/perlin-noise-algorithm
// Helper to get the constant vector for a corner
float2 GetConstantVector(int v)
{
		// v is the value from the permutation table
    const int h = v & 3; // equivalent to v % 4
    
    switch (h)
    {
        case 0:  return float2(1.0, 1.0);
        case 1:  return float2(-1.0, 1.0);
        case 2:  return float2(-1.0, -1.0);
        default: return float2(1.0, -1.0);
    }
}

// quick ease function (this is what Ken Perlin used)
float Fade(float t)
{
    return ((6.0f * t - 15.0f) * t + 10.0f) * t * t * t;
}

float Perlin2D(float x, float y, int permutation[512])
{
    const int X = (int)x & 255;
    const int Y = (int)y & 255;
    const float xf = x - (int)x;
    const float yf = y - (int)y;

	// Vectors pointing from grid points TO input point
    float2 topRight = float2(xf - 1.0f, yf - 1.0f);
    float2 topLeft = float2(xf, yf - 1.0f);
    float2 bottomRight = float2(xf - 1.0f, yf);
    float2 bottomLeft = float2(xf, yf);

	// Select a value in the array for each of the 4 corners
    float2 vecTopRight = GetConstantVector(permutation[permutation[X + 1] + Y + 1]);
    float2 vecTopLeft = GetConstantVector(permutation[permutation[X] + Y + 1]);
    float2 vecBottomRight = GetConstantVector(permutation[permutation[X + 1] + Y]);
    float2 vecBottomLeft = GetConstantVector(permutation[permutation[X] + Y]);

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

float FractalBrownianMotion(float x, float y, int numOctaves, int permutation[512])
{
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.005f;

    for (int octave = 0; octave < numOctaves; octave++)
    {
        result += amplitude * Perlin2D(x * frequency, y * frequency, permutation);

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return result;
}

#endif