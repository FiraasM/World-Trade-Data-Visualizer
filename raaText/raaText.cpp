#include "stdafx.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gl/glut.h>
#include "raaText.h"
#include <raaUtilities/raaUtilities.h>

GLYPHMETRICSFLOAT gmf[256];
unsigned int g_uiFontBase;

void buildFont()
{
	HFONT font;
	g_uiFontBase = glGenLists(256);

	font = CreateFontA(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE || DEFAULT_PALETTE, "Comic Sans MS");
	SelectObject(wglGetCurrentDC(), font);
	wglUseFontOutlines(wglGetCurrentDC(), 0, 255, g_uiFontBase, 0.0f, 0.2f, WGL_FONT_POLYGONS, gmf);

}

void killFont()
{
	glDeleteLists(g_uiFontBase, 256);
}

void outlinePrint(char* acString, bool bCentre)
{
	if(acString && strlen(acString))
	{
		float fLen = 0.0f;
		for (int i = 0; i < strlen(acString); i++) fLen += gmf[acString[i]].gmfCellIncX;

		glPushMatrix();
		if(bCentre) glTranslatef(-fLen*0.5f, 0.0f, 0.0f);
		glPushAttrib(GL_LIST_BIT);
		glListBase(g_uiFontBase);
		glCallLists((int)strlen(acString), GL_UNSIGNED_BYTE, acString);
		glPopAttrib();
		glPopMatrix();
	}
}


void drawText(char* acString, double x, double y, float colour[4]) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	utilitiesColourToMat(colour, 2.0f);
	glRasterPos2d(x, y);
	while (*acString) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *acString);
		acString++;
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);



}

