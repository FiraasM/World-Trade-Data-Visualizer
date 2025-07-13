#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "raaMaths.h"
#include "raaVector.h"

float degToRad( float f )
{
	return (f/180.0f)*cs_fPi;
}

float redToDeg( float f )
{
	return (f/cs_fPi)*180.0f;
}

void initMaths()
{
	if(!s_bMathsInit)
	{
		srand((unsigned int)time(0));
		s_bMathsInit=true;
	}
}

float randFloat( float fMin, float fMax)
{
	initMaths();
	return (((float)rand() / (float)RAND_MAX)*(fMax - fMin)) + fMin;
}

float mathsRadiusOfSphereFromVolume(float fVolume)
{
	return powf((fVolume*3.0f) / (4.0f*cs_fPi), 0.33f);
}

float mathsDimensionOfCubeFromVolume(float fVolume)
{
	return powf(fVolume, 0.33f);
}

float mathsRadiusOfConeFromVolume(float fVolume)
{
	return powf((fVolume*3.0f) / (2.0f*cs_fPi), 0.33f);
}

float mathsHeightOfConeFromVolumeAndRadius(float fVolume, float fRadius)
{
	return (3*fVolume) / (cs_fPi * fRadius * fRadius);
} 

bool pointIntersectWithSphere(float *fPointPosition, float *fSphereCenter, float fSphereRadius) {
	float boundsOffset = 2.5f;
	float distance = vecDistance(fPointPosition, fSphereCenter);
	return distance <= fSphereRadius + boundsOffset;

}

bool pointIntersectWithCube(float* fPointPosition, float* fSquareCenter, float fSquareSize) {
	float centerOffset = fSquareSize / 2.0f;
	float boundsOffset = 2.5f;

	float xMin = fSquareCenter[0] - centerOffset - boundsOffset;
	float xMax = fSquareCenter[0] + centerOffset + boundsOffset;

	float yMin = fSquareCenter[1] - centerOffset - boundsOffset;
	float yMax = fSquareCenter[1] + centerOffset + boundsOffset;

	float zMin = fSquareCenter[2] - centerOffset - boundsOffset;
	float zMax = fSquareCenter[2] + centerOffset + boundsOffset;

	return (xMin <= fPointPosition[0] && fPointPosition[0] <= xMax + boundsOffset) &&
		(yMin <= fPointPosition[1] && fPointPosition[1] <= yMax) &&
		(zMin <= fPointPosition[2] && fPointPosition[2] <= zMax);

}

bool pointIntersectWithCone(float* fPointPosition, float* fConeBaseCenter, float fConeRadius, float fConeHeight) {
	float fConeMaxHeight = fConeBaseCenter[1] + fConeHeight;
	if (fPointPosition[1] < fConeBaseCenter[1] || fPointPosition[1] > fConeMaxHeight) {
		return false;
	}

	float dx = fPointPosition[0] - fConeBaseCenter[0];
	float dz = fPointPosition[2] - fConeBaseCenter[2];

	float radialDistance = sqrtf((dx * dx) + (dz * dz));
	float coneRadiusAtY = (fConeRadius / fConeHeight) * fPointPosition[1];
	return radialDistance <= coneRadiusAtY;
}