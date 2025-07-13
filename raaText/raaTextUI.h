#pragma once
#ifdef _DEBUG
#pragma comment(lib,"raaTextD")
#else
#pragma comment(lib,"raaTextR")
#endif

#include <vector>
#include <raaComputerGraphicsAssignment1/raaVector.h>

typedef struct _raaTextInfo
{
	char* text;
	float colour[4];
	float offset[2];

} raaTextInfo;

typedef struct _raaTextUI
{
	float uiAnchor[2];
	raaVector* texts;


} raaTextUI;


typedef struct _newTest
{
	float uiPosition[2];
	std::vector<float> floats;

} newTest;

raaTextUI* createTextUI(float anchorX, float anchorY);
void addTextToUI(raaTextUI* UI, raaTextInfo* text);

raaTextInfo* createTextInfo(char* text, float R, float G, float B, float A, float offsetX, float offsetY);

void drawTextUI(raaTextUI* UI, float extraOffsetX, float extraOffsetY);
