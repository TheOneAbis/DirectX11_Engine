#pragma once

#include <string>

// Helpers for determining the actual path to the executable
std::wstring GetExePath();
std::wstring FixPath(const std::wstring& relativeFilePath);
std::string WideToNarrow(const std::wstring& str);
std::wstring NarrowToWide(const std::string& str);

class PerlinObject
{
private:

	int wrap;
	int* perlinTable;

public:

	/// <summary>
	/// Create a perlin object that contains perlin noise generation
	/// </summary>
	/// <param name="wrapValue">The maximum x and y value before the noise wraps back to the beginning</param>
	PerlinObject(int wrapValue);
	PerlinObject();
	PerlinObject& operator=(const PerlinObject& other);
	~PerlinObject();

	float Perlin2D(float x, float y);
	float FractalBrownianMotion(float x, float y, int numOctaves);
};
