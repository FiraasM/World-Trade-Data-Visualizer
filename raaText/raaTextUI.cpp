#include "stdafx.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/glut.h>
#include "raaText.h"
#include <raaUtilities/raaUtilities.h>

#include "raaTextUI.h"
#include <raaComputerGraphicsAssignment1/raaVector.h>


raaTextUI* createTextUI(float anchorX, float anchorY) {

	raaTextUI* newTextUI = (raaTextUI*)malloc(sizeof(raaTextUI));

	if (newTextUI == NULL) {
		return NULL;
	}

	newTextUI->texts = (raaVector*)malloc(sizeof(raaVector));
	initVector(newTextUI->texts, 4);

	float anchor[2] = { anchorX, anchorY };
	memcpy(newTextUI->uiAnchor, anchor, sizeof(anchor));

	return newTextUI;
}

void addTextToUI(raaTextUI* UI, raaTextInfo* text) {

	addElementToVector(UI->texts, text);
}


raaTextInfo* createTextInfo(char* text, float R, float G, float B, float A, float offsetX, float offsetY) {

	raaTextInfo* newText = (raaTextInfo*)malloc(sizeof(raaTextInfo));

	if (newText == NULL) {
		return NULL;
	}
	
	if (text == nullptr) {
		newText->text = (char*)malloc(101 * sizeof(char));
	}
	else {
		newText->text = text;
	}
	
	float colour[4] = { R, G, B, A };
	memcpy(newText->colour, colour, sizeof(colour));
	
	float offset[2] = { offsetX, offsetY };
	memcpy(newText->offset, offset, sizeof(offset));

	return newText;

}

void drawTextUI(raaTextUI* UI, float extraOffsetX, float extraOffsetY) {
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	for (int i = 0; i < UI->texts->size; ++i) {
		raaTextInfo* textInfo = (raaTextInfo * )(UI->texts->data[i]);
		float uiPosition[2] = { viewport[2] * UI->uiAnchor[0], viewport[3] * UI->uiAnchor[1]};
		drawText(textInfo->text, uiPosition[0] + textInfo->offset[0] + extraOffsetX, uiPosition[1] + textInfo->offset[1] + extraOffsetY, textInfo->colour);
	}
}