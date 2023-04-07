
#include <Windows.h>
#include <codecvt>
#include <locale>

#include "Helpers.h"
#include <DirectXMath.h>
#include <iostream>

using namespace DirectX;

// --------------------------------------------------------------------------
// Gets the actual path to this executable as a wide character string (wstring)
//
// - As it turns out, the relative path for a program is different when 
//    running through VS and when running the .exe directly, which makes 
//    it a pain to properly load external files (like textures & shaders)
//    - Running through VS: Current Dir is the *project folder*
//    - Running from .exe:  Current Dir is the .exe's folder
// - This has nothing to do with DEBUG and RELEASE modes - it's purely a 
//    Visual Studio "thing", and isn't obvious unless you know to look 
//    for it.  In fact, it could be fixed by changing a setting in VS, but
//    that option is stored in a user file (.suo), which is ignored by most
//    version control packages by default.  Meaning: the option must be
//    changed on every PC.  Ugh.  So instead, here's a helper.
// --------------------------------------------------------------------------
std::wstring GetExePath()
{
	// Assume the path is just the "current directory" for now
	std::wstring path = L".\\";

	// Get the real, full path to this executable
	wchar_t currentDir[1024] = {};
	GetModuleFileName(0, currentDir, 1024);

	// Find the location of the last slash charaacter
	wchar_t* lastSlash = wcsrchr(currentDir, '\\');
	if (lastSlash)
	{
		// End the string at the last slash character, essentially
		// chopping off the exe's file name.  Remember, c-strings
		// are null-terminated, so putting a "zero" character in 
		// there simply denotes the end of the string.
		*lastSlash = 0;

		// Set the remainder as the path
		path = currentDir;
	}

	// Toss back whatever we've found
	return path;
}


// ----------------------------------------------------
//  Fixes a relative path so that it is consistently
//  evaluated from the executable's actual directory
//  instead of the app's current working directory.
// 
//  See the comments of GetExePath() for more details.
// 
//  Note that this uses wide character strings (wstring)
//  instead of standard strings, as most windows API
//  calls require wide character strings.
// ----------------------------------------------------
std::wstring FixPath(const std::wstring& relativeFilePath)
{
	return GetExePath() + L"\\" + relativeFilePath;
}


// ----------------------------------------------------
//  Helper function for converting a wide character 
//  string to a standard ("narrow") character string
// ----------------------------------------------------
std::string WideToNarrow(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
}


// ----------------------------------------------------
//  Helper function for converting a standard ("narrow") 
//  string to a wide character string
// ----------------------------------------------------
std::wstring NarrowToWide(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

// ================================= //
// -- PerlinObject Implementation -- //
// ================================= //

PerlinObject::PerlinObject()
{
	this->wrap = 256;

	// Create array (permutation table) and shuffle it
	perlinTable = new int[wrap * 2];
	for (int i = 0; i < wrap; i++)
		perlinTable[i] = i;
	
	// Shuffle the array
	for (int e = wrap - 1; e > 0; e--) {
		const int index = e - 1 == 0 ? 0 : rand() % (e - 1);
		const int temp = perlinTable[e];

		perlinTable[e] = perlinTable[index];
		perlinTable[index] = temp;
	}

	// Duplicate the array
	for (int i = wrap; i < wrap * 2; i++)
		perlinTable[i] = perlinTable[i - wrap];
}

PerlinObject::PerlinObject(int* permutation, int wrap)
{
	this->wrap = wrap;

	// Copy array and duplicate it
	perlinTable = new int[wrap * 2];
	for (int i = 0; i < wrap; i++)
	{
		perlinTable[i] = permutation[i];
		perlinTable[i + wrap] = permutation[i];
	}
}

PerlinObject& PerlinObject::operator=(const PerlinObject& other)
{
	this->wrap = other.wrap;
	this->perlinTable = new int[wrap * 2];
	for (int i = 0; i < wrap * 2; i++)
		this->perlinTable[i] = other.perlinTable[i];
	return *this;
}

PerlinObject::~PerlinObject()
{
	delete[] perlinTable;
}

// Implementation of Perlin Noise. 
// Source Article: https://rtouti.github.io/graphics/perlin-noise-algorithm
float PerlinObject::Perlin2D(float x, float y)
{
	const int X = (int)x & (wrap - 1);	// Noise wraps at x = 256
	const int Y = (int)y & (wrap - 1);	// Noise wraps at y = 256
	const float xf = x - (int)x;
	const float yf = y - (int)y;

	// Vectors pointing from grid points TO input point
	XMFLOAT2 topRight = { xf - 1.0f, yf - 1.0f };
	XMFLOAT2 topLeft = { xf, yf - 1.0f };
	XMFLOAT2 bottomRight = { xf - 1.0f, yf };
	XMFLOAT2 bottomLeft = { xf, yf };

	// Helper to get the constant vector for a corner
	auto GetConstantVector = [](int v)
	{
		// v is the value from the permutation table
		const int h = v & 3; // equivalent to v % 4
		if (h == 0)
			return XMFLOAT2(1.0, 1.0);
		else if (h == 1)
			return XMFLOAT2(-1.0, 1.0);
		else if (h == 2)
			return XMFLOAT2(-1.0, -1.0);
		else
			return XMFLOAT2(1.0, -1.0);
	};

	// Select a value in the array for each of the 4 corners
	const XMFLOAT2 vecTopRight = GetConstantVector(perlinTable[perlinTable[X + 1] + Y + 1]);
	const XMFLOAT2 vecTopLeft = GetConstantVector(perlinTable[perlinTable[X] + Y + 1]);
	const XMFLOAT2 vecBottomRight = GetConstantVector(perlinTable[perlinTable[X + 1] + Y]);
	const XMFLOAT2 vecBottomLeft = GetConstantVector(perlinTable[perlinTable[X] + Y]);

	// Calculate dot products of each corner
	float dotTopRight;
	float dotTopLeft;
	float dotBottomRight;
	float dotBottomLeft;
	XMStoreFloat(&dotTopRight, XMVector2Dot(XMLoadFloat2(&topRight), XMLoadFloat2(&vecTopRight)));
	XMStoreFloat(&dotTopLeft, XMVector2Dot(XMLoadFloat2(&topLeft), XMLoadFloat2(&vecTopLeft)));
	XMStoreFloat(&dotBottomRight, XMVector2Dot(XMLoadFloat2(&bottomRight), XMLoadFloat2(&vecBottomRight)));
	XMStoreFloat(&dotBottomLeft, XMVector2Dot(XMLoadFloat2(&bottomLeft), XMLoadFloat2(&vecBottomLeft)));
	
	// quick ease function (this is what Ken Perlin used)
	auto Fade = [](float t) {return ((6.0f * t - 15.0f) * t + 10.0f) * t * t * t; }; 

	// Interpolate x and y to get final value
	const float u = Fade(xf);
	const float v = Fade(yf);
	float result;

	XMStoreFloat(&result, XMVectorLerp(
		XMVectorLerp(XMLoadFloat(&dotBottomLeft), XMLoadFloat(&dotTopLeft), v),
		XMVectorLerp(XMLoadFloat(&dotBottomRight), XMLoadFloat(&dotTopRight), v), u)
	);

	return result;
}

float PerlinObject::FractalBrownianMotion(float x, float y, int numOctaves) 
{
	float result = 0.0f;
	float amplitude = 1.0f;
	float frequency = 0.005f;

	for (int octave = 0; octave < numOctaves; octave++) {
		result += amplitude * Perlin2D(x * frequency, y * frequency);

		amplitude *= 0.5f;
		frequency *= 2.0f;
	}

	return result;
}