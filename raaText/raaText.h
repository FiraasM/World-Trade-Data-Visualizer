#pragma once
#ifdef _DEBUG
#pragma comment(lib,"raaTextD")
#else
#pragma comment(lib,"raaTextR")
#endif

void buildFont();
void killFont();
void outlinePrint(char* acString, bool bCentre=true);
void drawText(char* acString, double x, double y, float colour[4]);